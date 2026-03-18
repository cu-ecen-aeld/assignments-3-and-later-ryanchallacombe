/**
* @author Dan Walkes
* Time functions shared between multiple implementations
*/

#ifndef TIME_FUNCTIONS_SHARED_H
#define TIME_FUNCTIONS_SHARED_H

/*********/
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/************** GLOBALS *****************/

// #define RECVFILE        "/home/ryan/projects/sandbox/timer_thread/outfile.txt"
#define TIMER_PERIOD_SEC    10
#define TIMER_PERIOD_MS     0

struct thread_data
{
    int write_file_fd;              // fd of the file to write to
    pthread_mutex_t lock;           // a mutex to use when accessing the structure
};

/************** FUNCTION PROTOTYPES *****************/

int start_timestamp_wr_thread(int fd);
void timer_thread ( union sigval sigval );
bool setup_timer( int clock_id,
                         timer_t timerid, 
                         unsigned int timer_period_ms, 
                         unsigned int timer_period_sec,
                         struct timespec *start_time);

/*********/


/**
* Set @param result with the value of @param ts_start - @param ts_end, handling
* rollover, assuming @param ts_end is after @param ts_start
*/
static inline void timespec_diff_before_after( struct timespec *result,
                        const struct timespec *ts_start, const struct timespec *ts_end)
{
    result->tv_sec = ts_end->tv_sec - ts_start->tv_sec;
    if ( ts_end->tv_nsec < ts_start->tv_nsec ) {
        result->tv_nsec = 1000000000L - ts_start->tv_nsec + ts_end->tv_nsec;
        result->tv_sec--;
    } else {
        result->tv_nsec = ts_end->tv_nsec - ts_start->tv_nsec;
    }
}



/**
* Set @param result with a timespec representing @param float seconds
*/
static inline void seconds_to_timespec( struct timespec *result, float seconds)
{
    float nanoseconds;
    result->tv_sec = (time_t) seconds;
    nanoseconds = seconds - result->tv_sec;
    result->tv_nsec = (long) (nanoseconds * 1000000000L);
}

/**
* @return a float representation in seconds of the timespect at @param ts
*/
static inline float timespec_to_seconds( const struct timespec *ts )
{
    return ts->tv_sec + (((float)ts->tv_nsec) / 1000000000L);
}


/**
* Set @param result with the absolute time difference between @param time1 and @param time2, handling
* rollover.
*/
static inline void timespec_diff( struct timespec *result,
                        const struct timespec *time1, const struct timespec *time2)
{
	if( timespec_to_seconds(time2) >= timespec_to_seconds(time1) ) {
		timespec_diff_before_after(result,time1,time2);
	} else {
		timespec_diff_before_after(result,time2,time1);
	}
}

/**
* set @param result with @param ts_1 + @param ts_2
*/
static inline void timespec_add( struct timespec *result,
                        const struct timespec *ts_1, const struct timespec *ts_2)
{
    result->tv_sec = ts_1->tv_sec + ts_2->tv_sec;
    result->tv_nsec = ts_1->tv_nsec + ts_2->tv_nsec;
    if( result->tv_nsec > 1000000000L ) {
        result->tv_nsec -= 1000000000L;
        result->tv_sec ++;
    }
}
#endif
