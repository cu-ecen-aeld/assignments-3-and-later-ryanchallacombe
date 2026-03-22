/*
** aesdsocket.c
**
** Advanced Embedded Linux Programming: assignment 6 part 1
**		Updates to assignment 5 part 1
**
**		writer: ryan challacombe
**		date:	2/24/2026
*/

/************* 	TESTING *******************

Assignment 5 part 1
Verify the sample test script “sockettest.sh” successfully completes against your native compiled 
application each time your application is closed and restarted.  You can run this manually outside 
the ./full-test.sh script by:
•	Starting your aesdsocket application
•	Executing the sockettest.sh script from the assignment-autotest subdirectory:
		/home/ryan/projects/assignments-3-and-later-ryanchallacombe/assignment-autotest/test/assignment5
•	Stopping your aesdsocket application.

Assignment 6 part 1
3. Use the updated sockettest.sh script (in the assignment-autotest/test/assignment6 subdirectory). 
You can run this manually outside the `./full-test.sh` script by:
a. Starting your aesdsocket application
b. Executing the sockettest.sh script from the assignment-autotest subdirectory.
	/home/ryan/projects/assignments-3-and-later-ryanchallacombe/assignment-autotest/test/assignment6
c. Stopping your aesdsocket application.

VALGRIND checks:
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=/tmp/valgrind-out.txt ./aesdsocket

cat /tmp/valgrind-out.txt > /home/ryan/projects/assignments-3-and-later-ryanchallacombe/server/valgrindout.txt


************* 	TESTING *******************/

/************* 	ACCESS SYSLOG *******************

sudo cat /var/log/syslog

************* 	ACCESS SYSLOG *******************/

/************* 	REFERENCE INFO *******************

struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname

    struct addrinfo *ai_next;      // linked list, next node
};

int getaddrinfo(const char *node,   // e.g. "www.example.com" or IP
                const char *service,  // e.g. "http" or port number
                const struct addrinfo *hints,
                struct addrinfo **res);

int socket(int domain, int type, int protocol); 

int bind(int sockfd, struct sockaddr *my_addr, int addrlen);

int listen(int sockfd, int backlog); 

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 

struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
}; 

int recv(int sockfd, void *buf, int len, int flags);

************* 	REFERENCE INFO *******************/

#include "include/utils.h"
#include "time_functions_shared.h"


/**********************************
*	Defines
**********************************/

#define MAXDATASIZE		1024							// max number of bytes
#define RECVFILE		"/var/tmp/aesdsocketdata"		// file for storing received messages
//#define RECVFILE		"/home/ryan/projects/assignments-3-and-later-ryanchallacombe/server/output.txt"	// file for storing received messages

/**********************************
*	Globals
**********************************/

extern bool caught_signal;			// defined in utils.c
char *wr_file_path  = RECVFILE;
extern struct timespec ts;
extern struct tm tm;
extern char timestamp[100];
int num_added_to_list = 0;
int num_threads_created = 0;
int num_removed_from_list = 0;
int num_mallocd = 0;
int num_freed = 0;

/**********************************
*	Main
**********************************/
int main(int argc, char *argv[]) {


	/* Setup syslog */
	openlog(NULL, 0, LOG_USER);

	/* Setup a return value for the program */
	int rtn_val = 0;

	/* Let user know program is starting */
	const char *prog_name = argv[0];
	//syslog(LOG_DEBUG, "***** Starting program: %s\n", prog_name);
	printf("***** Starting program: %s\n", prog_name);


	/************************* Signal handler setup *************************/
	caught_signal = false;			// Signal handler flag
	struct sigaction new_action;	

    memset( &new_action, 0, sizeof(struct sigaction) );
    new_action.sa_handler = signal_handler;

    if( sigaction(SIGTERM, &new_action, NULL) != 0 ) {
        printf("Error %d (%s) registering for SIGTERM",errno,strerror(errno));
        exit_message();
        return -1;
    }
    if( sigaction(SIGINT, &new_action, NULL) ) {
        printf("Error %d (%s) registering for SIGINT",errno,strerror(errno));
        exit_message();
        return -1;
    }


    /************************* Setup write file *************************/
	// Remove it if it is already existing
	errno = 0;
	FILE *fp;
	if ( (fp = fopen(wr_file_path, "r")) != NULL ) {
		printf("Removing file: %s\n", wr_file_path);
		fclose(fp);
		if ( (remove(wr_file_path)) == -1)
			perror("remove");
	}

	// Create file
	//syslog( LOG_ERR, "Creating file %s.\n", wr_file_path);
	printf("Creating file: %s.\n", wr_file_path);
	errno = 0;
	int fd = creat( wr_file_path, S_IRWXU | S_IRWXG | S_IRWXO );
	// For some reason, file access is not correct, so close and reopen as below...
	close(fd);
	fd = open( wr_file_path, O_RDWR | O_APPEND);
	//printf("Write file fd in main(): %d\n", fd);
	
	// Check for errors
	if ( (errno != 0) || (fd == -1) ) {
		//char *error_str = strerror(errno);
		//syslog( LOG_ERR, "Error %d (%s) creating file %s.\n", errno, strerror(errno), wr_file_path );
		//syslog( LOG_ERR, "Returning with status = 1.\n" );
		printf( "Error %d (%s) creating file %s.\n", errno, strerror(errno), wr_file_path );
		printf( "Returning with status = -1.\n" );
		rtn_val = -1;
		goto DONE;
	}
	
	/************************* Socket *************************/
	// setup
	const char *port = "9000";
	struct addrinfo hints;
	struct addrinfo *servinfo;				// will point to the linked list of results


	// hints setup
	memset(&hints, 0, sizeof(hints));		// make sure that the struct is cleared
	hints.ai_family = AF_UNSPEC;			// don't care if ipv4 or ipv6
	hints.ai_socktype = SOCK_STREAM; 		// TCP stream sockets
	hints.ai_flags = AI_PASSIVE;     		// fill in my localhost IP for me

	// getaddrinfo call
	int ret;
	if ( (ret = getaddrinfo(NULL, port, &hints, &servinfo)) != 0 ) {
		// log error
		printf("getaddrinfo() call error: %s\n", gai_strerror(ret));
		exit_message();
        return -1;
	}

	// socket call
	int sockfd;
	errno = 0;
	if ( (sockfd = socket(servinfo->ai_family, servinfo-> ai_socktype, servinfo->ai_protocol)) == -1 ) {
		printf("socket() call error: %s\n", strerror(errno));
		exit_message();
        return -1;
	}
	//syslog(LOG_DEBUG, "***** Socket file set with descriptor: %d\n", sockfd);
	//printf("***** Socket file set with descriptor: %d\n", sockfd);

	// set SO_REUSEADDR on a socket to true (1):
	int optval = 1;
	socklen_t optlen = 0;
	if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) != 0) {
		perror("setsockopt");
	}

	getsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, &optlen);
	if (optval == 0) {
    	printf("Error enabling SO_REUSEADDR on sockfd\n");
	}

	// bind() call
	errno = 0;
	if ( (ret = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) == -1 ) {
		printf("bind() call error: %s\n", strerror(errno));
		rtn_val = -1;
		goto DONE;
	} 

	// setup for listening / accepting / while loop
	int backlog = 10;
	int spkr_fd;								// will hold fd of connection with speaker
	struct sockaddr_storage spkr_addr;			// struct to hold info about an accepted speaker
	socklen_t addr_size = sizeof(spkr_addr);	// get the size of the speaker struct
	char ipstr[INET_ADDRSTRLEN];  				// space to hold the IPv4 string


	/********** Daemon mode **********/
	
	// Daemon mode is invoked by argument 1 as '-d' when this application is invoked from command line
	pid_t pid;
	if (argc > 1) {			// check to ensure we got a command line argument
		if ( !strncmp("-d", argv[1], 2) ) {
			printf("Daemon mode invoked.\n");

			pid = fork();
			if ( pid == -1) {			// fork error
				perror("fork");
				return -1;
			} else if ( pid != 0 ) {	// pid = 0 is child pid

				// exit parent process
				printf("Exiting parent process\n");
				return 0;
			}
		}
	}

	/************* Initialize SLIST **************/
	// create data type for queue head
	SLIST_HEAD(head_s, sock_thread_data) head;

	// initialize the head before use
	SLIST_INIT(&head);

	/************* Setup vars **************/
	struct sock_thread_data *local_sock_thread_data = NULL;			// setup thread data struct
	pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;			// setup mutex that will be shared by the threads
	pthread_t my_thread = 0;	
	// printf("sizeof(struct sock_thread_data): %lu\n", sizeof(struct sock_thread_data));

	/************* Start Timestamp thread **************/
	// copy/pasting function start_timestamp_wr_thread() text here
	struct thread_data td;
    struct sigevent sev;
    timer_t timerid;
    memset(&td, 0, sizeof(struct thread_data));

    td.lock = fd_mutex;
    td.write_file_fd = fd;

    int clock_id = CLOCK_MONOTONIC;
    memset(&sev, 0, sizeof(struct sigevent));

    // Setup a call to timer_thread passing in the td structure as the sigev_value argument
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_value.sival_ptr = &td;            // point sival to thread_data
    sev.sigev_notify_function = timer_thread;   // function to run when the timer fires...

    if ( timer_create(clock_id, &sev, &timerid) != 0 ) {
        printf("Error %d (%s) creating timer!\n",errno,strerror(errno));
    } 

    struct timespec start_time;
    if ( ! setup_timer(clock_id, timerid, TIMER_PERIOD_MS, TIMER_PERIOD_SEC, &start_time) ) 
    {
        printf("Error setting up timer.\n");
    }

	/********** Loop **********/

	/************* MEM LEAK NOTE 3/22/2026 a6p1 **************/
    /*
		
		Valgrind shows a 40 byte mem leak happenning. I believe I tracked it down to somewhere in the while() loop. 
		40 bytes is the size of struct sock_thread_data. 
		I spent several hours looking for it with no luck

	*/
	/************* MEM LEAK NOTE 3/22/2026 a6p1 **************/
	while ( !caught_signal ) {

		// listen() call. This will block until a connection is established
		errno = 0;
		if ( (ret = listen(sockfd, backlog)) == -1 ) {
			//syslog(LOG_ERR, "listen() call error: %s\n", strerror(errno));
			printf( "listen() call error: %s\n", strerror(errno));
			//cleanup_func(sockfd, servinfo);
			//return -1;
			rtn_val = -1;
			goto DONE;
		}
		else {
			syslog(LOG_DEBUG, "Server set on port %s. Listening for client\n", port);
			//printf("Server set on port %s. Listening for client\n", port);
		}

		// accept() call
		errno = 0;
		if ( (spkr_fd = accept(sockfd, (struct sockaddr *)&spkr_addr, &addr_size)) == -1) {
			syslog(LOG_ERR, "accept() call error: %s\n", strerror(errno));
			//printf("accept() call error: %s\n", strerror(errno));
			//cleanup_func(sockfd, servinfo);	
			//return -1;
			rtn_val = -1;
			goto DONE;
		}

		// Print address of connected speaker
		getpeername(spkr_fd, (struct sockaddr *)&spkr_addr, &addr_size);
		inet_ntop(AF_INET, (struct sockaddr_in *)&spkr_addr, ipstr, INET_ADDRSTRLEN);
	    syslog(LOG_DEBUG, "Accepted connection from %s\n", ipstr);
	    //printf("Accepted connection from %s\n", ipstr);

		// malloc struct socket_thread_data
		local_sock_thread_data = malloc( sizeof( struct sock_thread_data ));
		if (local_sock_thread_data == NULL) 
	    {
	        printf("malloc error. Exiting\n");
	        //cleanup_func(sockfd, servinfo);
	        //return -1;
	        rtn_val = -1;
			goto DONE;
	    }
	    memset(local_sock_thread_data, 0, sizeof(struct sock_thread_data));
	    num_mallocd++;

		// insert into the linked list
	    SLIST_INSERT_HEAD(&head, local_sock_thread_data, nodes);
	    num_added_to_list++;

		// call function to start thread
		//printf("Starting thread...\n");
		if ( ! start_socket_thread(&my_thread, &fd_mutex, local_sock_thread_data, spkr_fd, fd) ) 
		{
			printf("Error starting thread. Exiting...");
			free(local_sock_thread_data);
			//cleanup_func(sockfd, servinfo);
	        //return -1;
	        rtn_val = -1;
			goto DONE;
		}

		num_threads_created++;
		syslog(LOG_DEBUG, "Thread started. Thread ID: %u\n", (unsigned int) my_thread);
		//printf("Thread started. Thread ID: %u\n", (unsigned int) my_thread);

		// reset local values, remain unchanged in linked list(????)
		//local_sock_thread_data = NULL;
		my_thread = 0;

		// loop through the linked list to look for completed threads
		SLIST_FOREACH(local_sock_thread_data, &head, nodes) 
		{
			// TODO: add logic to handle thread completed w/o success
			// if a thread is complete and successful, join it, remove it from linked list, then free it
			//  if complete but not successful.... do what??
			if ( local_sock_thread_data->thread_success || local_sock_thread_data->thread_complete )	
			{
				syslog(LOG_DEBUG, "Thread %u completed with thread_success = %d. Joining, removing, and freeing it...\n", 
					(unsigned int) local_sock_thread_data->tid, 
					(unsigned int) local_sock_thread_data->thread_success );


				ret = 0;
				ret = pthread_join(local_sock_thread_data->tid, NULL);
				if ( ret != 0) {
					printf("Attempt to join thread failed.\n");
				}

				// remove this element from the linked list and free it
				SLIST_REMOVE(&head, local_sock_thread_data, sock_thread_data, nodes);
				num_removed_from_list++;
				free(local_sock_thread_data);
				num_freed++;
			}
		}
	}	// end While loop

	/**************** Cleanup and Exit ****************/

	DONE:

	if ( close(fd) != 0 ) {
		printf("error closing write file\n");
	}
	if ( timer_delete(timerid) != 0 ) {
		printf("error deleting timer\n");
	}

	free(local_sock_thread_data);
	num_freed++;

	// unwind the linked list if it is not empty
	if ( !SLIST_EMPTY(&head) ) {
		syslog(LOG_DEBUG,"Freeing socket thread linked list.\n");
		struct sock_thread_data *e = NULL;
		e = malloc(sizeof(struct sock_thread_data));

		// free the elements from the queue
	    while ( !SLIST_EMPTY(&head) )
	    {
	        e = SLIST_FIRST(&head);
			ret = 0;
			ret = pthread_join(e->tid, NULL);
			if ( ret != 0) {
				printf("Attempt to join thread e failed.\n");
			}
	        SLIST_REMOVE_HEAD(&head, nodes);
	        //printf("********* SLIST_REMOVE e->tid: %u\n", (unsigned int) e->tid);
	        num_removed_from_list++;
	        free(e);
	        e = NULL;
	    }

	    free(e);
	    e = NULL;
	}

	cleanup_func(sockfd, servinfo);

	/*
	printf("num_threads_created: %d\n", num_threads_created);
	printf("num_added_to_list: %d\n", num_added_to_list);
	printf("num_removed_from_list: %d\n", num_removed_from_list);
	printf("num_mallocd: %d\n", num_mallocd);
	printf("num_freed: %d\n", num_freed);
	*/

	return rtn_val;
}