#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)


/*the thread begins life by executing a function that accepts a void pointer as its
sole argument and returns a void pointer as its return value.
*/
void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    // point a local struct pointer to the the value pointed to by the argument
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    // print thread ID
    //const pthread_t me = pthread_self ();
    //printf ("TID: %lu\n", (unsigned long) me);

    // waiting for predetermined time
    //printf("****** Waiting %d us before OBTAINING mutex\n", thread_func_args->us_wait_to_obtain_mutex);
    int rtn;
    errno = 0;
    rtn = usleep(thread_func_args->us_wait_to_obtain_mutex);
    if (rtn != 0) {
        perror("usleep");
        printf("*** usleep failed with error %d \n", rtn);
    }

    // obtain mutex
    int ret;
    ret = pthread_mutex_lock(thread_func_args->mutex);
    if (ret != 0) {
        printf("*** Error obtaining mutex with code %d\n", ret);
    }

    // waiting for predetermined time
    //printf("****** Waiting %d us before RELEASING mutex\n", thread_func_args->us_wait_to_release_mutex);
    errno = 0;
    rtn = usleep(thread_func_args->us_wait_to_release_mutex);
    if (rtn != 0) {
        perror("usleep");
        printf("*** usleep failed with error %d \n", rtn);
    }
    
    pthread_mutex_unlock(thread_func_args->mutex);

    // update the value to indicate successful thread execution
    thread_func_args->thread_complete_success = true;
    //printf("*** Thread completed successfully: %d\n", thread_func_args->thread_complete_success);

    return thread_param;
}

/**
* Start a thread which sleeps @param wait_to_obtain_ms number of milliseconds, then obtains the
* mutex in @param mutex, then holds for @param wait_to_release_ms milliseconds, then releases.
* The start_thread_obtaining_mutex function should only start the thread and should not block
* for the thread to complete.
* The start_thread_obtaining_mutex function should use dynamic memory allocation for thread_data
* structure passed into the thread.  The number of threads active should be limited only by the
* amount of available memory.
* The thread started should return a pointer to the thread_data structure when it exits, which can be used
* to free memory as well as to check thread_complete_success for successful exit.
* If a thread was started succesfully @param thread should be filled with the pthread_create thread ID
* coresponding to the thread which was started.
* @return true if the thread could be started, false if a failure occurred.
*/
bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    //printf("*** Begin of start_thread_obtaining_mutex() ***\n");

    struct thread_data *my_thread_data;
    my_thread_data = malloc( sizeof( struct thread_data ) );
    if (my_thread_data == NULL) {
        printf("malloc error. Exiting\n");
        return -1;
    }

    my_thread_data->mutex = mutex;
    my_thread_data->us_wait_to_obtain_mutex = 1000 * wait_to_obtain_ms;
    my_thread_data->us_wait_to_release_mutex = 1000 * wait_to_release_ms; 
    my_thread_data->thread_complete_success = false;

    // start thread with threadfunc as start point
    int ret;                // if the thread is successfully started this will be 0
    // void *ret_threadfunc;   // will store the value returned by threadfunc

    ret = pthread_create(thread, NULL, threadfunc, my_thread_data);
    if (ret != 0) {
        //printf("*** pthread_create failed with error %d creating thread\n", ret);
        return false;
    }

    return true;
}

