/*
** aesdsocket.c
**
** Advanced Embedded Linux Programming: assignment 5 part 1
**
**		writer: ryan challacombe
**		date:	11/21/2025
*/

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


/**********************************
*	Defines
**********************************/

#define MAXDATASIZE		1024						// max number of bytes
#define RECVFILE		"/var/tmp/aesdsocketdata"	// file for storing received messages

/**********************************
*	Globals
**********************************/

extern bool caught_signal;			// defined in utils.c

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
	syslog(LOG_DEBUG, "***** Starting program: %s\n", prog_name);


	/************************* Signal handler setup *************************/
	caught_signal = false;			// Signal handler flag
	struct sigaction new_action;	

    memset( &new_action, 0, sizeof(struct sigaction) );
    new_action.sa_handler = signal_handler;

    if( sigaction(SIGTERM, &new_action, NULL) != 0 ) {
        printf("Error %d (%s) registering for SIGTERM",errno,strerror(errno));
        rtn_val = -1;
        goto DONE;
    }
    if( sigaction(SIGINT, &new_action, NULL) ) {
        printf("Error %d (%s) registering for SIGINT",errno,strerror(errno));
        rtn_val = -1;
        goto DONE;
    }


    /************************* Remove write file if it exists *************************/
	char *wr_file_path = RECVFILE;
	errno = 0;
	FILE *fp;
	if ( (fp = fopen(wr_file_path, "r")) != NULL ) {
		printf("Removing file: %s\n", wr_file_path);
		fclose(fp);
		if ( (remove(wr_file_path)) == -1)
			perror("remove");
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
		rtn_val = -1;
		goto DONE;
	}

	// socket call
	int sockfd;
	errno = 0;
	if ( (sockfd = socket(servinfo->ai_family, servinfo-> ai_socktype, servinfo->ai_protocol)) == -1 ) {
		printf("socket() call error: %s\n", strerror(errno));
		rtn_val = -1;
		goto DONE;
	}
	syslog(LOG_DEBUG, "***** Socket file set with descriptor: %d\n", sockfd);

	// set SO_REUSEADDR on a socket to true (1):
	int optval = 1;
	socklen_t optlen;
	if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) != 0) {
		perror("setsockopt");
	}

	getsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, &optlen);
	if (optval != 0) {
    	printf("SO_REUSEADDR enabled on sockfd!\n");
	}

	// bind() call
	errno = 0;
	if ( (ret = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) == -1 ) {
		printf("bind() call error: %s\n", strerror(errno));
		rtn_val = -1;
		close(sockfd);
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
				rtn_val = -1;
				goto DONE;
			} else if ( pid != 0 ) {	// pid = 0 is child pid

				// exit parent process
				printf("Exiting parent process\n");
				rtn_val = 0;
				goto DONE;
			}
		}
	}
	

	/********** Loop **********/
	while ( !caught_signal ) {

		// listen() call
		errno = 0;
		if ( (ret = listen(sockfd, backlog)) == -1 ) {
			syslog(LOG_ERR, "listen() call error: %s\n", strerror(errno));
			rtn_val = -1;
			//close(sockfd);
			goto DONE;
		}

		printf("Server set on port %s. Listening for client\n", port);

		// accept() call
		errno = 0;
		if ( (spkr_fd = accept(sockfd, (struct sockaddr *)&spkr_addr, &addr_size)) == -1) {
			syslog(LOG_ERR, "accept() call error: %s\n", strerror(errno));
			rtn_val = -1;
			//close(sockfd);
			goto DONE;
		}

		// Print address of connected speaker
		getpeername(spkr_fd, (struct sockaddr *)&spkr_addr, &addr_size);
		inet_ntop(AF_INET, (struct sockaddr_in *)&spkr_addr, ipstr, INET_ADDRSTRLEN);
	    syslog(LOG_DEBUG, "Accepted connection from %s\n", ipstr);
	    printf("Accepted connection from %s\n", ipstr);

	    /************************* Receive data and write to file *************************/

	    // start receiving data
	    char *line;
	    int *return_flag, val = -1;		
	    return_flag = &val;		// set return_flag to a value that the function read_until_term doesn't use
	    errno = 0;	
		if ( (line = read_until_term(spkr_fd, '\n', return_flag)) == NULL) {
			printf("Error in read_until_term. Returning -1\n");
			rtn_val = -1;
			//close(sockfd);
			free(line);
			goto DONE;
		}

		//printf("***line read from the socket: %s\n", line);
		//printf("***return flag: %d\n", *return_flag);

		// Write to file
		if ( (ret = writer_func(wr_file_path, line)) != 0) {
			printf("writer_func returned: %d\n", ret);
			rtn_val = -1;
			free(line);
			goto DONE;
		}
		
		//printf("writer_func completed write\n");

		free(line);

		/**************** Read all data from the file and write back on the socket stream ****************/

		// open write file
		int wr_file_fd = open(wr_file_path, O_RDONLY);
		*return_flag = val;		// reset return flag value

		// loop to read from file and write to socket stream
		// line has been freed but we will reuse it here
		for (;;) {

			// TODO: review all the error handling
			line = read_until_term(wr_file_fd, '\n', return_flag);

			if ( *return_flag == 5 || *return_flag == 4 ) {		// reached EOF
				//printf("***return flag: %d\n", *return_flag);
				//printf("***reached EOF\n");
				break;
			} else if ( *return_flag == 1 || *return_flag == 2 ) {			// malloc or realloc failed
				printf("***return flag: %d\n", *return_flag);
				printf("***Memory allocation failure. Returning -1\n");
				rtn_val = -1;
				//close(sockfd);
				free(line);
				goto DONE;
			} else if ( *return_flag == 3 ) {
				printf("Error in read_until_term. Returning -1\n");
				rtn_val = -1;
				//close(sockfd);
				free(line);
				goto DONE;
			} else if ( *return_flag == 0 ){		// memory was sufficient and found a newline char
				//printf("***line: %s\n", line);
				//printf("***return flag: %d\n", *return_flag);

				// Write to socket
				int len, bytes_sent;
				len = strlen(line);
				if ( (bytes_sent = send(spkr_fd, line, len, 0)) != len ) {
					printf("Error, bytes transmission incomplete.\n");
					rtn_val = -1;
					//close(sockfd);
					free(line);
					goto DONE;
				}
				
				//printf("***sent: %d bytes\n", (uint) bytes_sent);
			} else {
				break;		// this should never be reached
			}

			free(line);

		}

		close(wr_file_fd);
		free(line);

		close(spkr_fd);
		syslog(LOG_DEBUG, "Closed connection from %s\n", ipstr);
		printf("Closed connection from %s\n", ipstr);
	}

	/**************** Cleanup and Exit ****************/

	
 
	DONE:

	close(sockfd);
	//free(line);
	freeaddrinfo(servinfo);		// free the linked list

	if ( caught_signal == true ) {
		syslog(LOG_DEBUG, "Caught signal, exiting\n");
		printf("Caught signal, exiting\n");
	}

	/* Let user know program is ending */
	syslog(LOG_DEBUG, "***** Exiting program %s with return value: %d\n", prog_name, rtn_val);
	printf("***** Exiting program %s with return value: %d\n", prog_name, rtn_val);

	return rtn_val;
}