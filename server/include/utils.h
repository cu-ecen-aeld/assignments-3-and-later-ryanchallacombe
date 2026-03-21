#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/queue.h>
#include <pthread.h>



#ifndef UTILS_H
#define	UTILS_H

#define INITIAL_BUFFER_SIZE		128

struct sock_thread_data {
    pthread_t tid;                              // thread ID
    pthread_mutex_t *mutex;                     // mutex that is shared between threads
    int client_fd;                              // file descriptor of the client this is connected to
    bool thread_complete;                       // completion flag
    bool thread_success;                        // completed with success
    int write_file_fd;              
    SLIST_ENTRY( sock_thread_data ) nodes;
};

int writer_func(const char *fpath, const char *buf);
void signal_handler ( int signal_number );
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


#endif