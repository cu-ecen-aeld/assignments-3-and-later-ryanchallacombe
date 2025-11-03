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

	//printf("***** Start of do_exec function *****\n");
	
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i < count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
	
	/* How to use execv:
	// int execv(const char *path, char *const argv[]);
	// the path needs to be the full path including the command to be executed
	// the first item in the vector argv[] needs to be the command itself without the path
	// the remaining items are arguments to be passed into the command
	*/
	
	// copy command into a new array because we don't need command[0] in the second execv argument
	// we instead need the command (basename) of command[0]
	// Since we are going to be looping through command[] it will change the value (pointer movement?)
	// so we copy the full command path to cpath. 
	// After this section, we dont' need command anymore
	char *cmd[count];
	char *cpath = command[0];
	cmd[0] = basename(command[0]);
	//printf("*** cmd[0]=%s\n", cmd[0]);
	for (int i = 1; i <= count; i++)
	{
		cmd[i] = command[i];
		//printf("*** cmd[%d]=%s\n", i, cmd[i]);	
	}

	// Clean up arg list. This is required after va_start()
    va_end(args);
   	
   	//printf("**** do_exec: Preparing to fork... first 2 execv params are: %s and %s\n", cpath, cmd[0]);
	
	int status;
	pid_t pid;

	pid = fork();
	if (pid == -1) {		// fork error
		perror("fork");
		return false;
	}
	
	// in the child process fork() returns 0, so the following is executed in the child
	else if (pid == 0) {
		if ( execv(cpath, cmd) == -1) {		// execv error
			perror("EXECV error in do_exec"); 
			return false;
		}
	}
	
	
	if (waitpid(pid, &status, 0) == -1) {	// wait error
		perror("***** waitpid");
		return false;
	}
	//else if (!WIFEXITED(status)) {
	else if (WEXITSTATUS(status) != 0) {	
		//printf("******* Command executed by execv returned: %d. Going to DONE\n", WEXITSTATUS(status));
		return false;
	}
	else {
		// if we get to this point, no errors have occured, so set return value to true
		return true;
	}
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
// 	references for dup2: 	https://stackoverflow.com/a/13784315/1446624 
// 							https://stackoverflow.com/questions/1720535/practical-examples-use-dup-or-dup2
bool do_exec_redirect(const char *outputfile, int count, ...)
{

	//printf("***** Start of do_exec_redirect function\n");	
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
        //printf("command[%d]=%s\n", i, command[i]);
    }
    command[count] = NULL;

	// copy command into a new array because we don't need command[0] in the second execv argument
	// we instead need the command (basename) of command[0]
	char *cmd[count];
	char *cpath = command[0];
	cmd[0] = basename(command[0]);
	
	// It was discovered that in the loop below traversing through command[] moves the command ptr
	for (int i = 1; i <= count; i++) {
		cmd[i] = command[i];
		//printf("*** cmd[%d]=%s\n", i, cmd[i]);	
	}

	// Clean up arg list. This is required after va_start()
    va_end(args);
	
	int status;
	pid_t pid;
	
	// open the file passed into the function
	int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
	
	pid = fork();
	if (pid == -1) {		// fork error
		perror("fork");
		return false;
	}
	
	// in the child process fork() returns 0, so the following is executed in the child
	else if (pid == 0) {	
		//printf("**** first 2 execv params are: %s and %s\n", command[0], cmd[0]);
		
		// this sets fd 1 (STDOUT) to the same file descriptor as fd which is the file name passed into the function
		// so from henceforth, if this code doesnt' have an error, anything going to STDOUT goes to the file name
		// This actually opens the file with fd = 1
		if (dup2(fd, STDOUT_FILENO) < 0) {
			perror("dup2"); 
			return false; 
		} 
		
		// we are now done with the old fd so we can close it
		close(fd);
    
    	if (execv(cpath, cmd) == -1) {		// execv error
			perror("EXECV error in do_exec_redirect"); 
			return false;
		}
	}
	
	if (waitpid(pid, &status, 0) == -1) {	// wait error
		perror("***** waitpid");
		return false;
	}
	//else if (!WIFEXITED(status)) {
	else if (WEXITSTATUS(status) != 0) {	
		//printf("******* Command executed by execv returned: %d. Going to DONE\n", WEXITSTATUS(status));
		return false;
	}
	else {
		// if we get to this point, no errors have occured, so set return value to true
		return true;
	}
}
