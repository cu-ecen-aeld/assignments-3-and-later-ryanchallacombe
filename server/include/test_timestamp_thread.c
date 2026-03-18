#include "time_functions_shared.h"
// #include "timestamp_thread.h"

/************** GLOBALS *****************/

extern struct timespec ts;
extern struct tm tm;
extern char timestamp[100];


int main(void)
{
    char *fpath  = RECVFILE;
    int fd;
    
    /* Open file and error check */
    fd = open( fpath, O_RDWR | O_APPEND);
    if ( fd == -1 ) {       
        // Create file
        printf("Creating file %s.\n", fpath);
        errno = 0;
        fd = creat( fpath, S_IRWXU | S_IRWXG | S_IRWXO );
        
        // Check for errors
        if ( (errno != 0) || (fd == -1) ) {
            char *error_str = strerror(errno);
            printf("ERRNO string: %s\n", error_str );
            printf("Error creating file %s.\n", fpath );
            printf("Returning with status = -1.\n" );
            return -1;
        }
    }

    int ret = start_timestamp_wr_thread(fd);
    if ( ret == 0 )
    {   
        
        /*int num_loops = 60;
        volatile unsigned int sleep_sec = 1;
        for(int i = 0; i < num_loops; i++) {
            sleep( sleep_sec );
        }*/
        for(;;) {
            for(int i=0; i<1000; i++)
            {}
        }


    }
    else
    {
        printf("Failed to start thread.\n");
    }

    close(fd);
    //timer_delete(timerid);
    return ret;

}