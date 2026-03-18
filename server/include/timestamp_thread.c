/*

    timestamp_thread
        functions for starting a thread that prints a timestamp
        to a given file descriptor (fd) in a given period. 
        The fd and the period are both defined by preprocessor macros 
        in this implementation.

        Based on timer_thread.c by Dan Walkes located here:
        /home/ryan/projects/sandbox/aesd-lectures/lecture9

*/

#include "time_functions_shared.h"

/************** GLOBALS *****************/

struct timespec ts;
struct tm tm;
char timestamp[100];

// moved to header file
/*
struct thread_data
{
    int write_file_fd;              // fd of the file to write to
    pthread_mutex_t lock;           // a mutex to use when accessing the structure
};*/

/************** FUNCTION PROTOTYPES *****************/

void timer_thread ( union sigval sigval );
bool setup_timer( int clock_id,
                         timer_t timerid, 
                         unsigned int timer_period_ms, 
                         unsigned int timer_period_sec,
                         struct timespec *start_time);
int start_timestamp_wr_thread(int fd);


/************** FUNCTIONS *****************/

/**
* A thread which runs every timer_period_ms milliseconds
* Assumes timer_create has configured for sigval.sival_ptr to point to the
* thread data used for the timer
*/
void timer_thread ( union sigval sigval )
{
    struct thread_data *td = (struct thread_data*) sigval.sival_ptr;
    errno = 0;
    if ( pthread_mutex_lock(&td->lock) != 0 ) {
        printf("Error %d (%s) locking thread data!\n",errno,strerror(errno));
    } else {

        // ref time_functions by Dan Walkes
        if ( clock_gettime(CLOCK_REALTIME, &ts) != 0 ) {
            printf("Error in clock_gettime call.\n");
        }

        if ( gmtime_r(&ts.tv_sec, &tm) == NULL ) {
            printf("Error calling gmtime_r with time %ld\n", ts.tv_sec);
        } 
        else 
        {
            //printf("gmtime_r done\n");
            // print in RFC 822 compilant date format
            if( strftime(timestamp, sizeof(timestamp), "timestamp:%a, %d %b %Y %T\n", &tm) == 0 ) {
                printf("Error converting string with strftime\n");
            } 
            else {
                //printf("%s\n", timestamp);
                
                ssize_t bytes_written;
                int buf_bytes = strlen(timestamp);
                bytes_written = write(td->write_file_fd, timestamp, buf_bytes);
                if ( bytes_written != buf_bytes ) {
                    printf("Error writing to file.\n");
                    printf("Number of bytes in buffer = %i\n", buf_bytes);
                    printf("Bytes written (returned from write()) = %li\n", bytes_written);
                }
            }
        }

        if ( pthread_mutex_unlock(&td->lock) != 0 ) {
            printf("Error %d (%s) unlocking thread data!\n",errno,strerror(errno));
        }
    }
}

/**
* Setup the timer at @param timerid (previously created with timer_create) to fire
* every @param timer_period_ms milliseconds, using @param clock_id as the clock reference.
* The time now is saved in @param start_time
* @return true if the timer could be setup successfuly, false otherwise
*/
bool setup_timer( int clock_id,
                         timer_t timerid, 
                         unsigned int timer_period_ms, 
                         unsigned int timer_period_sec,
                         struct timespec *start_time)
{
    bool success = false;
    if ( clock_gettime(clock_id, start_time) != 0 ) {
        printf("Error %d (%s) getting clock %d time\n",errno,strerror(errno),clock_id);
    } else {
        struct itimerspec itimerspec;
        memset(&itimerspec, 0, sizeof(struct itimerspec));
        itimerspec.it_interval.tv_sec = timer_period_sec;
        itimerspec.it_interval.tv_nsec = timer_period_ms * 1000000;
        timespec_add(&itimerspec.it_value, start_time, &itimerspec.it_interval);
        if( timer_settime(timerid, TIMER_ABSTIME, &itimerspec, NULL ) != 0 ) {
            printf("Error %d (%s) setting timer\n",errno,strerror(errno));
        } else {
            success = true;
        }
    }
    
    return success;
}



int start_timestamp_wr_thread(int fd)
{

    struct thread_data td;
    struct sigevent sev;
    bool success = false;
    timer_t timerid;
    memset(&td, 0, sizeof(struct thread_data));
    //printf("sizeof(struct thread_data): %lu\n", sizeof(struct thread_data));

    if ( pthread_mutex_init(&td.lock, NULL) != 0 ) {
        printf("Error %d (%s) initializing thread mutex!\n",errno,strerror(errno));
    } else {

        //printf("td->lock = %d\n", (int) td.lock);
        td.write_file_fd = fd;
        int clock_id = CLOCK_MONOTONIC;
        memset(&sev,0,sizeof(struct sigevent));
        //printf("sizeof(struct sigevent): %lu\n", sizeof(struct sigevent));

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
        else {
            success = true;
        }
    }
    return success ? 0 : -1;
}

