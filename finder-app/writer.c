#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <libgen.h>
#include <dirent.h>
#include <syslog.h>
#include <string.h>

// Not a requirement, but need to fix the length issue later.
char command_mkdir[255] = "mkdir -p ";

int main(int argc, char **argv)
{
    /**
     * Requirement: Open the connection to the system logger.
    */
    openlog(argv[0],0,LOG_USER);

    /**
     * Accepts the following arguments:
     *  Arg1: First argument is a full path to a file (including filename) on the filesystem, referred to below as writefile
     *  Arg2: Second argument is a text string which will be written within this file, referred to below as writestr
     *
     * If not specified:
     * Exits with value 1 error and print statements if any of the arguments above were not specified
     */
    if (argc < 3)
    {
        printf("ERROR: This application requires 2 arguments: %s\n", "Exit Code 1");
        syslog(LOG_ERR, "This application requires 2 arguments: %s\n", "Exit Code 1");
        closelog();
        return 1;
    }
    else if (argv[1] == NULL || argv[2] == NULL || argc > 3)
    {
        if (argv[1] == NULL)
        {
            printf("ERROR: This application requires a path & filename: %s\n", "Exit Code 1");
            syslog(LOG_ERR, "This application requires a path & filename: %s\n", "Exit Code 1");
            closelog();
            return 1;
        }
        else if (argv[2] == NULL)
        {
            printf("ERROR: This application requires content %s\n", "Exit Code 1");
            syslog(LOG_ERR, "This application requires content %s\n", "Exit Code 1");
            closelog();
            return 1;
        }
        else
        {
            printf("ERROR: Too many arguments, %s\n", "Exit Code 1");
            syslog(LOG_ERR, "Too many arguments, %s\n", "Exit Code 1");
            closelog();
            return 1;
        }
    }
    else
    {
        /**
         * Requirement: First argument is a full path to a file (including filename) on the filesystem.
         * Spliting Directory, and Filename.
         */
            const char *path = argv[1];
            const char *filepath = dirname(strdup(path));;
            const char *filename = basename(strdup(path));

        /**
         * Requirement: Text string which will be written within this file.
         */
        const char *content = argv[2];

        /**
         * Optional Requirement: Create Directory if it doesn't exist
        */
       DIR* direcroty = opendir(filepath);
       if (errno == ENOENT)
       {
        fprintf(stderr, "ERROR: No such directory %s, error %d\n", filepath, errno);
        syslog(LOG_DEBUG, "No such directory %s, error %d\n", filepath, errno);
        printf("Creating Directory\n");
        strcat(command_mkdir, filepath);
        system(command_mkdir);
        syslog(LOG_DEBUG, "New directory created %s\n", filepath);
       }
       else
       {
        printf("CONTINUE: Directory exists\n");
       }
    
        /**
         * Open File with fopen which takes filename, and mode.
         * Returns a file pointer or NULL on failure.
         * The w mode will disgard file contents on open.
         */
        FILE *file = fopen(path, "wb");

        if (file == NULL)
        {
            fprintf(stderr, "ERROR: Failed to open file %s in mode wd, error %d\n", filename, errno);
            syslog(LOG_ERR, "Failed to open file %s in mode wd, error %d\n", filename, errno);
            closelog();
            return 1;
        }

        else
        {
            /**
             * Close file with fclose which takes File pointer.
             * Returns 0 or EOF on failure
             */
            printf("SUCCESS: The file %s was written in mode wb.\n", filename);
            syslog(LOG_DEBUG, "The file %s was written in mode wb.\n", filename);

            /**
             * Requirement: Write argumement 2, to file
             */
            if (content == NULL)
            {
                fprintf(stderr, "ERROR: No content passed to writer, error %d\n", errno);
                syslog(LOG_ERR, "No content passed to writer, error %d\n", errno);
                closelog();
                return 1;
            }
            else
            {
                fputs(content, file);
                printf("SUCCESS: The content %s was written to file.\n", content);
                syslog(LOG_DEBUG, "The content %s was written to file.\n", content);
            }

            if (fclose(file) == EOF)
            {
                fprintf(stderr, "ERROR: Failed to close file %s in mode wd, error %d\n", filename, errno);
                syslog(LOG_ERR, "Failed to close file %s in mode wd, error %d\n", filename, errno);
                closelog();
                return 1;
            }
            else
            {
                printf("SUCCESS: The file %s was closed. \n", filename);
                syslog(LOG_DEBUG, "The file %s was closed. \n", filename);
            }
        }
        closedir(direcroty);
    }

    /**
     * Requirement:
     * Creates a new file with name and path writefile with content writestr, overwriting any existing file and creating the
     * path if it doesn’t exist. Exits with value 1 and error print statement if the file could not be created.
     * Example writer.sh /tmp/aesd/assignment1/sample.txt
     */
    closelog();
    return 0;
}