/***********************************************
#
#	UC Boulder ECE 5305
#
#	Assignment# 2: Linux System Programming
#
#	File: writer.c
#
#	Author: Ryan Challacombe
#	Date: 10/11/2025
#
#
***********************************************/

/***********************************************

The program duplicates the function of writer.sh from assignment 1 with the following exceptions
1. One difference from the write.sh instructions in Assignment 1:  You do not need to make your "writer" utility create directories which do not exist.  You can assume the directory is created by the caller.

2. Setup syslog logging for your utility using the LOG_USER facility.

3. Use the syslog capability to write a message “Writing <string> to <file>” where <string> is the text string written to file (second argument) and <file> is the file created by the script.  This should be written with LOG_DEBUG level.

4. Use the syslog capability to log any unexpected errors with LOG_ERR level.


The instructions for writer.sh are below

# Accepts the following arguments: the first argument is a full path to a file (including filename) 
# on the filesystem, referred to below as writefile; the second argument is a text string which will 
# be written within this file, referred to below as writestr

# Exits with value 1 error and print statements if any of the arguments above were not specified

# Creates a new file with name and path writefile with content writestr, overwriting any existing file 
# and creating the path if it doesn’t exist. Exits with value 1 and error print statement if the file 
# could not be created.

***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

int main(int argc, char *argv[])
{	
	/* Setup syslog */
	openlog(NULL, 0, LOG_USER);
	
	if ( argc < 3 ) {
		syslog( LOG_ERR, "ERROR: invalid number of arguments.\n");
		syslog( LOG_ERR, "Total number of arguments should be 2.\n");
		syslog( LOG_ERR, "The order of the arguments should be:\n");
		syslog( LOG_ERR, "\t1. The full path to the write file (including filename)\n");
		syslog( LOG_ERR, "\t2. The string to be written to the write file");
		exit(1);
	}
	
	const char *fpath = argv[1];
	const char *buf = argv[2];
	int fd;
	ssize_t bytes_written;
	int buf_bytes = strlen(buf);
	
	/* Open file and error check */
	fd = open( fpath, O_RDWR );
	if ( fd == -1 ) {
		// Error opening file, possibly due to non-existent file
		syslog( LOG_ERR, "Couldn't open file %s. Possibly non-existent.\n", fpath);
		
		// Create file
		syslog( LOG_ERR, "Creating file %s.\n", fpath);
		errno = 0;
		fd = creat( fpath, S_IRWXU | S_IRWXG | S_IRWXO );
		
		// Check for errors
		if ( (errno != 0) || (fd == -1) ) {
			char *error_str = strerror(errno);
			syslog( LOG_ERR, "ERRNO string: %s\n", error_str );
			syslog( LOG_ERR, "Error creating file %s.\n", fpath );
			syslog( LOG_ERR, "Exiting with status = 1.\n" );
			exit(1);
		}
	}
	
	/* Write to file and error check */
	syslog( LOG_DEBUG, "Writing %s to %s\n", buf, fpath );
	bytes_written = write( fd, buf, buf_bytes );
	if ( bytes_written != buf_bytes ) {
		syslog( LOG_ERR, "Error writing to file %s.\n", fpath);
		syslog( LOG_ERR, "Number of bytes in buffer = %i\n", buf_bytes);
		syslog( LOG_ERR, "Bytes written (returned from write()) = %li\n", bytes_written);
		syslog( LOG_ERR, "Exiting with status = 1.\n");
		exit(1);
	}
	
	close( fd );
		
	return 0;
}

