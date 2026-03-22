#include "utils.h"


/////////////////////////////
//		Function prototypes
/////////////////////////////

int writer_func(const char *fpath, const char *buf);
void signal_handler(int signal_number);
char *read_until_term(int fd, const char term, int *rtn_flag);
ssize_t readLine(int fd, void *buffer, size_t n);
void cleanup_func(int socket_fd, struct addrinfo *servinfo);
void exit_message(void);
void *sock_thread_func(void *thread_param);
bool start_socket_thread(pthread_t *thread, 
    pthread_mutex_t *mutex, 
    struct sock_thread_data *my_thread_data,
    int spkr_fd,
    int wr_file_fd);
int writer_func_fd(int fd, const char *buf);


/**********************************
*   Globals
**********************************/

bool caught_signal;
extern char *wr_file_path;

/////////////////////////////
//		Function definitions
/////////////////////////////

// sock_thread_func
// function to call when starting the socket thread
void *sock_thread_func(void *thread_param) {

    // point a local struct pointer to the the value pointed to by the argument
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct sock_thread_data *thread_func_args = (struct sock_thread_data*) thread_param;
    int ret = 0;
    int spkr_fd = thread_func_args->client_fd;

    int fd = thread_func_args->write_file_fd;

    /************************* Receive data *************************/

    char *line;
    int *return_flag, no_flag_val = -1;     
    return_flag = &no_flag_val;     // set return_flag to a value that the function read_until_term doesn't use
    errno = 0;  
    if ( (line = read_until_term(spkr_fd, '\n', return_flag)) == NULL) {
        printf("Error in read_until_term. Returning -1\n");
        free(line);
        //cleanup_func(sockfd, servinfo);
        //return -1;
    } else  {
        //printf("***line read from the socket: %s\n", line);
        //printf("***return flag: %d\n", *return_flag);

        /************************* Obtain mutex and write to file *************************/
        errno = 0;
        if ( pthread_mutex_lock(thread_func_args->mutex) != 0 ) {
            printf("Error %d (%s) locking thread data!\n", errno, strerror(errno));
        } else {
            // Write to file
            //printf("Mutex lock completed.\n");
            ret = lseek(fd, (off_t) 0, SEEK_SET); 
            //printf("lseek returns: %d\n", ret);
            if ( (ret = writer_func_fd(thread_func_args->write_file_fd, line)) != 0) {
                printf("writer_func returned: %d\n", ret);
                free(line);
                //cleanup_func(sockfd, servinfo);
                //return -1;
            } else {
                //printf("writer_func completed write\n");
                free(line);
            }

            /**************** Read all data from the file and write back on the socket stream ****************/
            // open write file
            //int wr_file_fd = open(wr_file_path, O_RDONLY)
            return_flag = &no_flag_val;             // reset return flag value

            // set file position
            lseek(thread_func_args->write_file_fd, (off_t) 0, SEEK_SET); 

            // loop to read from file and write to socket stream
            // line has been freed but we will reuse it here
            for (;;) {

                //printf("Write file fd in sock_thread_func: %d\n", thread_func_args->write_file_fd);
                line = read_until_term(thread_func_args->write_file_fd, '\n', return_flag);

                if ( *return_flag == 5 || *return_flag == 4 ) {             // reached EOF, break out of the loop
                    //printf("***return flag: %d\n", *return_flag);
                    //printf("***reached EOF\n");
                    thread_func_args->thread_success = true; 
                    break;
                } else if ( *return_flag == 1 || *return_flag == 2 ) {      // malloc or realloc failed
                    printf("***return flag: %d\n", *return_flag);
                    printf("***Memory allocation failure.\n");
                    free(line);
                    break;
                    //cleanup_func(sockfd, servinfo);
                    //return -1;
                } else if ( *return_flag == 3 ) {
                    printf("Error in read_until_term. Return flag: %d\n", *return_flag);
                    free(line);
                    break;
                    //cleanup_func(sockfd, servinfo);
                    //return -1;
                } else if ( *return_flag == 0 ){                            // memory was sufficient and found a newline char
                    //printf("***line: %s\n", line);
                    //printf("***return flag: %d\n", *return_flag);

                    // Write to socket
                    int len, bytes_sent;
                    len = strlen(line);
                    if ( (bytes_sent = send(spkr_fd, line, len, 0)) != len ) {
                        printf("Error, bytes transmission incomplete.\n");
                        free(line);
                        //cleanup_func(sockfd, servinfo);
                        //return -1;
                    }
                    
                    //printf("***sent: %d bytes\n", (uint) bytes_sent);
                } else {
                    break;      // this should never be reached
                } 

                free(line);
                line = NULL;
            }

            if ( pthread_mutex_unlock(thread_func_args->mutex) != 0 ) {
                errno = 0;
                printf("Error %d (%s) unlocking thread data!\n",errno,strerror(errno));
            } else {
                //printf("Mutex unlock completed.\n");
                thread_func_args->thread_success = true;               
            }
        }
    }
    thread_func_args->thread_complete = true;
    return thread_param;
}

bool start_socket_thread(pthread_t *thread, 
    pthread_mutex_t *mutex, 
    struct sock_thread_data *my_thread_data,
    int spkr_fd,
    int wr_file_fd)
{
    my_thread_data->tid = 0;        // this will get populated when the thread begins
    my_thread_data->mutex = mutex;
    my_thread_data->thread_complete = false;
    my_thread_data->thread_success = false;
    my_thread_data->client_fd = spkr_fd;
    my_thread_data->write_file_fd = wr_file_fd;

    int ret = 0;                
    ret = pthread_create(thread, NULL, sock_thread_func, my_thread_data);
    if (ret != 0) {
        printf("*** pthread_create failed with error %d creating thread\n", ret);
        return false;
    }

    my_thread_data->tid = *thread; 

    return true;
}


// Print a message to inform of exit
void exit_message(void)
{
    /* Let user know program is ending */
    syslog(LOG_DEBUG, "***** Exiting aesdsocket program\n");
    //printf("***** Exiting aesdsocket program *****\n");
    return;
}

// Close socket and addr info linked list, inform if signal was caught, print exit message
void cleanup_func(int socket_fd, struct addrinfo *servinfo)
{
    if ( close(socket_fd) != 0 ) {
        printf("error closing socket\n");
    }
    freeaddrinfo(servinfo);     // free the server linked list

    if ( caught_signal == true ) {
        syslog(LOG_DEBUG, "Caught signal, exiting\n");
        //printf("Caught signal, exiting\n");
    }

    exit_message();
    return;
}

// Same as writer_func() but accepts an open file descriptor, fd
// fd must be already open with WR access at a minimum
// the fd is left open after the function returns
// it's up to the caller to open and close the fd
int writer_func_fd(int fd, const char *buf)
{   
    // ASSUME SYSLOG IS OPEN

    //syslog( LOG_DEBUG, "*** Starting writer_func ***\n" );
    
    //int fd;
    ssize_t bytes_written;
    int buf_bytes = strlen(buf);
    
    /* Write to file and error check */
    syslog( LOG_DEBUG, "Writing %s to file\n", buf);
    bytes_written = write( fd, buf, buf_bytes );
    if ( bytes_written != buf_bytes ) {
        syslog( LOG_ERR, "Error writing to file.\n");
        syslog( LOG_ERR, "Number of bytes in buffer = %i\n", buf_bytes);
        syslog( LOG_ERR, "Bytes written (returned from write()) = %li\n", bytes_written);
        syslog( LOG_ERR, "Returning from writer_func with status = -1.\n" );
        return -1;
    }
    
    syslog( LOG_DEBUG, "*** Returning from writer_func with status = 0 ***\n" );
    return 0;
}

int writer_func(const char *fpath, const char *buf)
{	
	// ASSUME SYSLOG IS OPEN

	syslog( LOG_DEBUG, "*** Starting writer_func ***\n" );
	
	int fd;
	ssize_t bytes_written;
	int buf_bytes = strlen(buf);
	
	/* Open file and error check */
	fd = open( fpath, O_RDWR | O_APPEND);
	if ( fd == -1 ) {		
		// Create file
		syslog( LOG_ERR, "Creating file %s.\n", fpath);
		errno = 0;
		fd = creat( fpath, S_IRWXU | S_IRWXG | S_IRWXO );
		
		// Check for errors
		if ( (errno != 0) || (fd == -1) ) {
			char *error_str = strerror(errno);
			syslog( LOG_ERR, "ERRNO string: %s\n", error_str );
			syslog( LOG_ERR, "Error creating file %s.\n", fpath );
			syslog( LOG_ERR, "Returning with status = 1.\n" );
			return 1;
		}
	}
	
	/* Write to file and error check */
	syslog( LOG_DEBUG, "Writing %s to %s\n", buf, fpath );
	bytes_written = write( fd, buf, buf_bytes );
	if ( bytes_written != buf_bytes ) {
		syslog( LOG_ERR, "Error writing to file %s.\n", fpath);
		syslog( LOG_ERR, "Number of bytes in buffer = %i\n", buf_bytes);
		syslog( LOG_ERR, "Bytes written (returned from write()) = %li\n", bytes_written);
		syslog( LOG_ERR, "Returning from writer_func with status = -1.\n" );
		return -1;
	}
	
	close( fd );
	
	syslog( LOG_DEBUG, "*** Returning from writer_func with status = 0 ***\n" );
	return 0;
}


// TODO
// Gracefully exits when SIGINT or SIGTERM is received, completing any open connection operations, 
// closing any open sockets, and deleting the file /var/tmp/aesdsocketdata.
// Logs message to the syslog “Caught signal, exiting” when SIGINT or SIGTERM is received.

void signal_handler(int signal_number)
{
    // Save a copy of errno so we can restore it later.  See https://pubs.opengroup.org/onlinepubs/9699919799/
    int errno_saved = errno;

    if ( (signal_number == SIGINT) || (signal_number == SIGTERM) )
        caught_signal = true;

    errno = errno_saved;
}


// read_until_term()
// 
// Reads from a file descriptor until EOF or the specified terminal character is found
//
// Dynamically allocates and reallocates buffer memory until EOF or the specified terminal character is found
// Returns a pointer to the buffer
// Returns NULL on error or no characters read
//
// It's up to the caller to free() this pointer when done with it.
//
// 	Notes:
// 	1. The specified terminal character *is* included in the returned buffer
//	2. When the caller passes in a null character '\0' the function will read until EOF
//  3. Return flags can be used to determine exactly when happened after the function returns
// 
// Credits/references: The below were used as reference material in developing this function	
//						beej's 'readline' function: https://beej.us/guide/bgc/html/split/manual-memory-allocation.html
//						Michael Kerrisk's 'readLine' function: https://man7.org/tlpi/code/online/dist/sockets/read_line.c.html
//
//  Return flags
//  0 = terminal character found
//  1 = malloc failed
//  2 = realloc failed
//  3 = read() error
//  4 = reached EOF and no bytes were read
//  5 = reached EOF and some bytes were read


char *read_until_term(int fd, const char term, int *rtn_flag)
{

    ssize_t num_read;      	// # of bytes fetched by last read()
    size_t tot_read = 0;       	// Total bytes read so far
    int offset = 0;   		// Index of next char that goes in the buffer
    int bufsize = INITIAL_BUFFER_SIZE;  
    char *buf;        		
    char c;            		// The character we've read in

    buf = malloc(bufsize);  // Allocate initial buffer

    if (buf == NULL) {   		// Error check
        printf("malloc failed\n");
        *rtn_flag = 1; 
        return NULL;
    }

    do {
    	// Check if we're out of room in the buffer accounting
        // for the extra byte for the NUL terminator
    	if (offset == bufsize -1) {
    		bufsize = 2 * bufsize;			// double the size

    		char *new_buf = realloc(buf, bufsize);

    		 if (new_buf == NULL) {
                free(buf);   // On error, free and bail
                printf("realloc failed\n");
                *rtn_flag = 2; 
                return NULL;
            }

            buf = new_buf;  // Successful realloc

    	}

        // set file position
        //int ret = lseek(fd, (off_t) 0, SEEK_SET); 
        //printf("lseek returns: %d\n", ret);
        //ret = fcntl(fd, F_GETFL);
        //printf("fcntl returns: %#06x\n", ret);

    	// read a single character 
    	errno = 0;
    	num_read = read(fd, &c, 1);
        //printf("read_until_term read c: %c\n", c);
        // printf("read returned: %d\n", (uint)num_read);
    	if ( num_read == -1) {
    		if (errno == EINTR)         // Interrupted --> restart read()
                continue;
            else {
                printf("read errno %d (%s) in read_until_term\n", errno, strerror(errno));
            	free(buf);
                *rtn_flag = 3; 
                return NULL;              // Some other error 
            }
    	}
    	else if (num_read == 0)	{		// EOF
    		if (tot_read == 0) {		// no bytes read, return null
    			free(buf);
                *rtn_flag = 4; 
                return NULL;
    		}
    		else						// some bytes have been read, break loop and add '\0'
                *rtn_flag = 5; 
    			break;	
    	}		
 
    	buf[offset] = c;  // Add the byte onto the buffer
    	//printf("buf[offset]: %c\n", buf[offset] );
    	offset++;
    	//printf("new offset: %d\n", offset);
    	tot_read++;
        *rtn_flag = 0; 

    }  while ( buf[offset-1] != term );

    // We hit newline or EOF...

    //printf("Total chars read: %d\n", (int)tot_read);

    // Shrink to fit
    if (offset < bufsize - 1) {  // If we're short of the end
        char *new_buf = realloc(buf, offset + 1); // +1 for NUL terminator

        // If successful, point buf to new_buf;
        // otherwise we'll just leave buf where it is
        if (new_buf != NULL)
            buf = new_buf;
    }

    // Add the NUL terminator
    buf[offset] = '\0';

    return buf;
}




/* Read characters from 'fd' until a newline is encountered. If a newline
  character is not encountered in the first (n - 1) bytes, then the excess
  characters are discarded. The returned string placed in 'buf' is
  null-terminated and includes the newline character if it was read in the
  first (n - 1) bytes. The function return value is the number of bytes
  placed in buffer (which includes the newline character if encountered,
  but excludes the terminating null byte). */

// Credit: https://man7.org/tlpi/code/online/dist/sockets/read_line.c.html

ssize_t readLine(int fd, void *buffer, size_t n)
{
    ssize_t numRead;                    // # of bytes fetched by last read()
    size_t totRead;                     // Total bytes read so far
    char *buf;
    char ch;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = buffer;                       // No pointer arithmetic on "void *" 

    totRead = 0;
    for (;;) {
        numRead = read(fd, &ch, 1);

        if (numRead == -1) {
            if (errno == EINTR)         // Interrupted --> restart read()
                continue;
            else
                return -1;              // Some other error 

        } else if (numRead == 0) {      // EOF
            if (totRead == 0)           // No bytes read; return 0
                return 0;
            else                        // Some bytes read; add '\0' 
                break;

        } else {                        // 'numRead' must be 1 if we get here 
            if (totRead < n - 1) {     // Discard > (n - 1) bytes 
                totRead++;
                *buf++ = ch;
            }

            if (ch == '\n')
                break;
        }
    }

    *buf = '\0';
    return totRead;
} 

// Remove and free each node from the socket thread linked list
//void unwind_stll(struct sock_thread_data s_td, struct sock_thread_data *head, struct sock_thread_data nodes) 
/*
void unwind_stll( struct head_s *func_head ) 
{

    if (func_head->)

    SLIST_HEAD(head_s, sock_thread_data) head;
    head = func_head;

    head = head;    
 
    SLIST_FOREACH(s_td, head, nodes) 
    {

        // remove this element from the linked list
        SLIST_REMOVE(head, s_td, sock_thread_data, nodes);
        free(s_td);
    }
}*/