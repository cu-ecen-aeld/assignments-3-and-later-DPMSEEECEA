#include "systemcalls.h"
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

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

    /**
     * Requirement: call system passing the parameter cmd
    */
    int return_value = system(cmd);
    if (return_value != 0)
    {
        fprintf(stderr, "ERROR: The command %s exited with the following message, error %d\n", cmd, errno);
        syslog(LOG_ERR, "The command %s exited with the following message, error %d\n", cmd, errno);
        return false;
    }
    else
    {
        printf("SUCCESS: The command %s was executed successfully.\n", cmd);
        syslog(LOG_DEBUG, "The command %s was executed successfully.\n", cmd);
        return true;
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
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
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
    va_end(args);
    /**
    * Requirement: Execute a system command by calling fork, execv(), and wait instead of system (see LSP page 161).
    */
    pid_t process_id;
    int status;

    process_id = fork();
    if (process_id < 0)
    {
        fprintf(stderr, "ERROR: Failed to create child process, error %d\n", errno);
        syslog(LOG_ERR, "ERROR: Failed to create child process, error %d\n", errno);
        return -1;
    } 
    else if (process_id == 0)
    {
        pid_t child_process_id;
        child_process_id = getpid();
        printf("SUCCESS: The child process %d is still running.\n", child_process_id);
        syslog(LOG_DEBUG, "SUCCESS: The child process %d is still running.\n", child_process_id);
        execv(command[0], command);
        printf("MESSAGE: replaced existing process with  %s \n", command[0]);
        exit(EXIT_FAILURE);
        return false;
    }
    else do
    {
        if (waitpid(process_id, &status, 0) == -1)
        {
            fprintf(stderr, "ERROR: wait error, error %d\n", errno);
            syslog(LOG_ERR, "ERROR: wait error, error %d\n", errno);
        }
        else if (WIFEXITED(status))
        {
            printf("SUCCESS: The child process exited normally %d . \n", WEXITSTATUS(status));
            syslog(LOG_DEBUG, "SUCCESS: The child process exited normally %d . \n", WEXITSTATUS(status));
            if (WEXITSTATUS(status) == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }     

    } while (process_id == 0);
    
    return false;
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
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    int file_descriptor = open(outputfile, O_WRONLY | O_CREAT, 0600);
    pid_t process_id;
    int status;

    process_id = fork();
    if (process_id < 0)
    {
        fprintf(stderr, "ERROR: Failed to create child process, error %d\n", errno);
        syslog(LOG_ERR, "ERROR: Failed to create child process, error %d\n", errno);
        return -1;
    } 
    else if (process_id == 0)
    {
        pid_t child_process_id;
        child_process_id = getpid();
        printf("SUCCESS: The child process %d is still running.\n", child_process_id);
        syslog(LOG_DEBUG, "SUCCESS: The child process %d is still running.\n", child_process_id);
        if (dup2(file_descriptor, 1) < 0)
        {
            return false;
        }
        execv(command[0], command);
        printf("MESSAGE: replaced existing process with  %s \n", command[0]);
        exit(EXIT_FAILURE);
        close(file_descriptor);
    }
    else do
    {
        if (waitpid(process_id, &status, 0) == -1)
        {
            fprintf(stderr, "ERROR: wait error, error %d\n", errno);
            syslog(LOG_ERR, "ERROR: wait error, error %d\n", errno);
        }
        else if (WIFEXITED(status))
        {
            printf("SUCCESS: The child process exited normally %d . \n", WEXITSTATUS(status));
            syslog(LOG_DEBUG, "SUCCESS: The child process exited normally %d . \n", WEXITSTATUS(status));
            if (WEXITSTATUS(status) == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }     

    } while (process_id == 0);
    close(file_descriptor);
    return false;
}