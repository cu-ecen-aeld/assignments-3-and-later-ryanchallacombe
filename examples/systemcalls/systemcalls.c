#define _XOPEN_SOURCE
#include "systemcalls.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <libgen.h>
#include <fcntl.h>
#include <errno.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
	
	int ret;
	
	ret = system(cmd);
	if ( WIFEXITED(ret) ) {
		//printf("Command executed and terminated normal with exit status=%d\n", WEXITSTATUS(ret));
		return true;
	}
	else {
		//printf("System call failure. Return status=%d\n", ret);
		return false;
	}

    //return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i < count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

	/*
	 * TODO:
	 *   Execute a system command by calling fork, execv(),
	 *   and wait instead of system (see LSP page 161).
	 *   Use the command[0] as the full path to the command to execute
	 *   (first argument to execv), and use the remaining arguments
	 *   as second argument to the execv() command.
	 *
	*/
	
	/* How to use execv:
	// int execv(const char *path, char *const argv[]);
	// the path needs to be the full path including the command to be executed
	// the first item in the vector argv[] needs to be the command itself without the path
	// the remaining items are arguments to be passed into the command
	*/
	
	// copy command into a new array because we don't need command[0] in the second execv argument
	// we instead need the command (basename) of command[0]
	char *cmd[count];
	cmd[0] = basename(command[0]);
	//printf("*** cmd[0]=%s\n", cmd[0]);
	for (int i = 1; i <=	 count; i++)
	{
		cmd[i] = command[i];
		//printf("*** cmd[%d]=%s\n", i, cmd[i]);	
	}

	bool ret = false;		
	int status;
 	int ret_execv = 0;
	pid_t pid;
	
	pid = fork();
	if (pid == -1) {
		goto DONE;
	}
	
	// in the child process fork() returns 0, so the following is executed in the child
	else if (pid == 0) {
		ret_execv = execv(command[0], cmd);
		
		if (ret_execv == -1) {
			// printf("******* EXECV returned -1. Going to DONE\n");
			goto DONE;
		}
		
		
		// execv won't return unless there is a failure
		// in which case it returns with -1
		// the goto statment is only executed if execv returns
		perror("execv");
		//printf("******* EXECV returned without executing. Going to DONE\n");
		goto DONE;
	}
	
	if (waitpid(pid, &status, 0) == -1)
		goto DONE;
	else if (WEXITSTATUS(status) != 0) {
		//printf("******* Command executed by execv returned: %d. Going to DONE\n", WEXITSTATUS(status));
		goto DONE;
	}
	
	// if we get to this point, no errors have occured, so set return value to true
	ret = true;
	
	DONE:
	
	// Clean up arg list. This is required after va_start()
    va_end(args);
    
    //printf("*** Returning (1=true, 0=false): %d\n", ret );

    return ret;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
        printf("command[%d]=%s\n", i, command[i]);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];
    printf("command[count]=%s\n", command[count]);
    printf("command[0]=%s\n", command[0]);

/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

	// 	references: 	https://stackoverflow.com/a/13784315/1446624 
	// 					https://stackoverflow.com/questions/1720535/practical-examples-use-dup-or-dup2
	// 	
	
	// copy command into a new array because we don't need command[0] in the second execv argument
	// we instead need the command (basename) of command[0]
	char *cmd[count];
	char *cpath = command[0];
	cmd[0] = basename(command[0]);
	
	// It was discovered that in the loop below traversing through command[] moves the command ptr
	for (int i = 1; i <= count; i++) {
		cmd[i] = command[i];
		printf("*** cmd[%d]=%s\n", i, cmd[i]);	
	}
	 printf("command[0] after cmd loop =%s\n", command[0]);

	bool ret = false;		
	int status;
 	int ret_execv = 0;
	pid_t pid;
	
	// open the file passed into the function
	int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
	
	// do this to ovoid duplicate print outs
	fflush(stdout);
	
	pid = fork();
	if (pid == -1) {
		goto DONE;
	}

	
	// in the child process fork() returns 0, so the following is executed in the child
	else if (pid == 0) {	
		printf("**** first 2 execv params are: %s and %s\n", command[0], cmd[0]);
		
		// this sets fd 1 (STDOUT) to the same file descriptor as fd which is the file name passed into the function
		// so from henceforth, if this code doesnt' have an error, anything going to STDOUT goes to the file name
		// This actually opens the file with fd = 1
		if (dup2(fd, STDOUT_FILENO) < 0) {
			perror("dup2"); 
			goto DONE; 
		} 
		
		// we are now done with the old fd so we can close it
		close(fd);
    
    	errno = 0;
		ret_execv = execv(command[0], cmd);
		ret_execv = execv(cpath, cmd);

		if (ret_execv == -1) {
			perror("EXECV error"); 
			//printf("******* EXECV returned -1. \n");
			//printf("******* Going to DONE\n");
			goto DONE;
		}
		
		
		// execv won't return unless there is a failure
		// in which case it returns with -1
		// the goto statment is only executed if execv returns
		perror("execv");
		//printf("******* EXECV returned without executing. Going to DONE\n");
		goto DONE;
	}
	
	//close(fd);
	
	if (waitpid(pid, &status, 0) == -1)
		goto DONE;
	else if (WEXITSTATUS(status) != 0) {
		//printf("******* Command executed by execv returned: %d. Going to DONE\n", WEXITSTATUS(status));
		goto DONE;
	}
	
	// if we get to this point, no errors have occured, so set return value to true
	ret = true;
	
	DONE:
	
	// Clean up arg list. This is required after va_start()
    va_end(args);
    
    //printf("*** Returning (1=true, 0=false): %d\n", ret );	
    close(fd);
    return ret;

}
