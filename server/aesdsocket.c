#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <syslog.h>
#include <libgen.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


/*
    GLOBALS: Specify Port & Path
    Requirement:
    Reference: https://beej.us/guide/bgnet/html/
    NOTE:FIX Tagging assignment-5-part-1
*/
#define port_numebr "9000"
#define queue_buffer 64
const char* absolute_path = "/var/tmp/aesdsocketdata";
struct sigaction signal_action;
const char* filepath = "/var/tmp/";
const char* filename = "aesdsocketdata";

/*
    FUNCTION: Create working direcroty
    Requirement: Create file at path "/var/tmp/aesdsocketdata";
*/
int create_working_directory()
{
    int status = 1;
    char command_mkdir[20] = "mkdir -p ";

    /*
        Requirement: append date to a file, create file if it does not exist
    */
    //const char* filename = basename(strdup(absolute_path));

    /**
        Requirement: Create Directory if it doesn't exist
    */
    //DIR* direcroty = opendir(filepath);
    if (errno == ENOENT)
    {
        fprintf(stderr, "ERROR: No such directory %s, error %d\n", filepath, errno);
        syslog(LOG_DEBUG, "No such directory %s, error %d\n", filepath, errno);
        printf("Creating Directory\n");
        strcat(command_mkdir, filepath);
        system(command_mkdir);
        syslog(LOG_DEBUG, "New directory created %s\n", filepath);
        status = 0;
    }
    else
    {
        printf("CONTINUE: Directory exists\n");
        status = 0;
    }
    return status;
}
/*
    Requirement: Process inboud data.
*/
int get_message_queue_size(int connections_file_descriptor, char buffer[queue_buffer]){
    int sizeof_buffer = queue_buffer;
    int new_sizeof_buffer = sizeof_buffer;
    int byte_count = 0;
    int stop = 0; 
    int grow = 0;


    while (byte_count <= new_sizeof_buffer && stop == 0)
    {
        char *receive_data = ""; 
        new_sizeof_buffer = (queue_buffer*grow);
        receive_data = (char *)malloc(new_sizeof_buffer);
        bzero(receive_data, new_sizeof_buffer);
        grow++;
        byte_count = recv(connections_file_descriptor, receive_data, new_sizeof_buffer, MSG_PEEK);
        for (int i = 0; i <= new_sizeof_buffer-1; i++)
        {
            if (receive_data[i] == '\n')
            {
                stop = 1;
            }        
        }
        bzero(receive_data, new_sizeof_buffer);
        free(receive_data);        

    }

return new_sizeof_buffer;
}
/*
    FUNCTION: Write data to file in bianary more.
    Requirement: Write the data to file
*/
int write_data_to_file(char* content)
{
    int status = 1;
    /**
     * Open File with fopen which takes filename, and mode.
     * Returns a file pointer or NULL on failure.
     * The a mode will append to an exsiting file or create a new file.
     * Requirement: The
     */
    FILE* file = fopen(absolute_path, "a+b");

    /*
    * Requirement: append date to a file, create file if it does not exist
    */

    if (file == NULL)
    {
        fprintf(stderr, "ERROR: Failed to open file %s in mode wd, error %d\n", filename, errno);
        syslog(LOG_ERR, "Failed to open file %s in mode wd, error %d\n", filename, errno);
        closelog();
        status = 1;
    }
    else
    {
        fputs(content, file);
        fclose(file);
        status = 0;
    }

    return status;
}

/*
    FUNCTION: Get the data from the connecting client.
    Requirement: This is were we will actually recive the data not using the preview flag
*/
int receive_data_from_client(int connections_file_descriptor, char buffer[queue_buffer])
{
    int status = 0;
    int sizeof_buffer = get_message_queue_size(connections_file_descriptor, buffer);


        char *receive_data = ""; 
        receive_data = (char *)malloc(sizeof_buffer);
        bzero(receive_data, sizeof_buffer);
        recv(connections_file_descriptor, receive_data, sizeof_buffer, 0);
        /*
            Requirement: Write data to file.
        */
        write_data_to_file(receive_data);
        bzero(receive_data, sizeof_buffer);        
        free(receive_data);        
    

    return status;
}

/*
    FUNCTION: Send response back to the client.
    Requirement: Read the file and send the data to the client
*/
int send_data_to_client(int connections_file_descriptor)
{
    int status = 1;
    /**
     * Open File with fopen which takes filename, and mode.
     * Returns a file pointer or NULL on failure.
     * The a mode will append to an exsiting file or create a new file.
     * Requirement: Read the file and return the content to the client.
     */
    FILE* file = fopen(absolute_path, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "ERROR: Failed to open file %s in mode r, error %d\n", filename, errno);
        syslog(LOG_ERR, "ERROR: Failed to open file %s in mode r, error %d\n", filename, errno);
        closelog();
        status = 1;
    }
    else
    {
        //Let's get the file size
        int length = 0;
        fseek(file, 0, SEEK_END);
        length = ftell(file);

        //SET the positon to the start of the file issues with lseek
        fseek(file, 0, SEEK_SET);
        int data_element;

        //NOTE: Allocate taransmit data mem
        char *transmit_data = ""; 
        transmit_data = (char *)malloc(length*sizeof(char));

        //NOTE: loop through each char in the file from starting postion. 
        for (int i = 0; i < length; i++)
        {
            data_element = fgetc(file);
            transmit_data[i] = (char)data_element;
        }
        //NOTE: Send data to client.
        send(connections_file_descriptor, transmit_data , length, 0);

        //NOTE: return mem to system
        free(transmit_data);
        status = 0;
        fclose(file);
    }

    return status;
}

/*
    FUNCTION: stop server and cleanup
    Requirement:
*/
int stop_server()
{
    int status = 1;
    char command[] = "rm -fr ";
    strcat(command, absolute_path);
    system(command);
    printf("\nSERVER INFO: Server is shutting down, removing artifacts...\n");
    fprintf(stderr, "SERVER ERROR: Signal action, error: %d\n", errno);
    syslog(LOG_ERR, "SERVER ERROR: Signal action, error: %d\n", errno);
    closelog();
    exit(1);
    return status;
}

/*
    FUNCTION: Setup Signal Handler
    Requirement: Setup sginal handler for SIGINT and SIGTERM
*/
void signal_handler(int signal)
{
    //FROM Example: waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;

    if (signal == SIGINT)
    {
        stop_server();
        printf("SERVER INFO: Srver received a SIGINT and is shutting down...");
    }
    else if (signal == SIGTERM)
    {
        stop_server();
        printf("SERVER INFO: Srver received a SIGTERM and is shutting down...");
    }
}

/*
    FUNCTION: Get address IPv4 and IPv6
    Requirement: get socket address, IPv4 or IPv6:
*/
void* get_address(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
/*
    FUNCTION: Start listing and accepting connections
    Requirement:
*/
int start_server()
{
    int socket_file_descriptor;
    int connections_file_descriptor;
    // NOTE: rsi, Resulting Server Information
    int rsi;
    struct addrinfo address_information, * server_information, * ptr_server_information;
    struct sockaddr_storage session_address_information;
    struct sigaction signal_action;
    socklen_t ancillary_buffer;
    char layer3_address[INET6_ADDRSTRLEN];
    char buffer[queue_buffer] = "";
    int true = 1;
    int status = 1;

    /*
        START OF: Get Address Info
        NOTE: Load address structure, getaddrinfo()
    */
    //NOTE: Initialize Structure.
    memset(&address_information, 0, sizeof address_information);
    //NOTE: Allow IPv4 & IPv6.
    address_information.ai_family = AF_UNSPEC;
    //NOTE: Set the socket to TCP.
    address_information.ai_socktype = SOCK_STREAM;
    //NOTE: Bind to the host IPv4 or IPv6 address.
    address_information.ai_flags = AI_PASSIVE;
    //SYSLOG, Console Output
    printf("INFO: Initialized address structure, IPv4&IPv6 allowed, protocol TCP\n");
    syslog(LOG_DEBUG, "INFO: Initialized address structure, IPv4&IPv6 allowed, protocol TCP\n");
    //NOTE: rsi, Resulting Server Information
    rsi = getaddrinfo(NULL, port_numebr, &address_information, &server_information);
    //NOTE: If rsi is 0 something went wrong...
    if (rsi != 0) {
        fprintf(stderr, "ERROR: This application failed to get address infromation, error: %s\n", gai_strerror(rsi));
        syslog(LOG_ERR, "ERROR: This application failed to get address infromation, error: %s\n", gai_strerror(rsi));
        closelog();
        return 1;
    }
    printf("INFO: Address structure created, IPv4&IPv6 allowed, protocol TCP\n");
    syslog(LOG_DEBUG, "INFO: Address structure created, IPv4&IPv6 allowed, protocol TCP\n");
    /**
         END of Get Address Info.
    */


    /*
        START OF: Create Socket
        NOTE: creat a new socket
    */
    // loop through all the results and bind to the first we can
    for (ptr_server_information = server_information; ptr_server_information != NULL; ptr_server_information = ptr_server_information->ai_next)
    {
        //NOTE: Get socket file descriptor
        socket_file_descriptor = socket(ptr_server_information->ai_family, ptr_server_information->ai_socktype, ptr_server_information->ai_protocol);

        if (socket_file_descriptor == -1)
        {
            fprintf(stderr, "SERVER ERROR: Failed to get socket file descriptor, error: %d\n", errno);
            syslog(LOG_ERR, "SERVER ERROR: Failed to get socket file descriptor, error: %d\n", errno);
            continue;
        }
        //NOTE: Set socket options
        if (setsockopt(socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) == -1)
        {
            fprintf(stderr, "SERVER ERROR: Failed to set socket options, error: %d\n", errno);
            syslog(LOG_ERR, "SERVER ERROR: Failed to set socket options, error: %d\n", errno);
            closelog();
            exit(1);
        }
        //NOTE: Attempt to bind
        if (bind(socket_file_descriptor, ptr_server_information->ai_addr, ptr_server_information->ai_addrlen) == -1)
        {
            close(socket_file_descriptor);
            fprintf(stderr, "SERVER ERROR: Failed to bind to socket, error: %d\n", errno);
            syslog(LOG_ERR, "SERVER ERROR: Failed to bind to socket, error: %d\n", errno);
            continue;
        }

        break;
    }
    //NOTE: Free structure when done
    freeaddrinfo(server_information);

    if (ptr_server_information == NULL)
    {
        fprintf(stderr, "SERVER ERROR: Failed to bind to socket, error: %d\n", errno);
        syslog(LOG_ERR, "SERVER ERROR: Failed to bind to socket, error: %d\n", errno);
        closelog();
        exit(1);
    }
    //NOTE: Start listening for inbound connections
    if (listen(socket_file_descriptor, queue_buffer) == -1)
    {
        fprintf(stderr, "SERVER ERROR: Failed to listen, error: %d\n", errno);
        syslog(LOG_ERR, "SERVER ERROR: Failed to listen, error: %d\n", errno);
        closelog();
        exit(1);
    }

    //NOTE: Setup signal action
    signal_action.sa_handler = &signal_handler;
    sigemptyset(&signal_action.sa_mask);
    signal_action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &signal_action, NULL);
    sigaction(SIGTERM, &signal_action, NULL);

    if (sigaction(SIGCHLD, &signal_action, NULL) == -1)
    {
        fprintf(stderr, "SERVER ERROR: Signal action, error: %d\n", errno);
        syslog(LOG_ERR, "SERVER ERROR: Signal action, error: %d\n", errno);
        closelog();
        exit(1);
    }

    printf("SERVER INFO: Waiting for clinet connections...\n");
    create_working_directory();
    status = 0;

    while (true)
    {
        ancillary_buffer = sizeof session_address_information;
        connections_file_descriptor = accept(socket_file_descriptor, (struct sockaddr*)&session_address_information, &ancillary_buffer);

        if (connections_file_descriptor == -1)
        {
            printf("INFO: Nothing to see here... no one is calling\n");
            syslog(LOG_DEBUG, "INFO: Nothing to see here... no one is calling\n");
            continue;
        }

        inet_ntop(session_address_information.ss_family, get_address((struct sockaddr*)&session_address_information), layer3_address, sizeof layer3_address);
        /*
            Requirement: Log all client connections. "Accepted connection from XXXX"
        */
        printf("SERVER INFO: clinet connection established from IP %s\n", layer3_address);
        syslog(LOG_DEBUG, "SERVER INFO: Accepted connection from %s\n", layer3_address);

        if (!fork())
        {
            // NOTE: Remove listener 
            close(socket_file_descriptor);
            /*
                Requirement: Read data from client input, until a new line character is received.
            */
            receive_data_from_client(connections_file_descriptor, buffer);
            /*
                Requirement: Read the file at absolute_path and send the contents to the client.
            */
            send_data_to_client(connections_file_descriptor);

            close(connections_file_descriptor);
            exit(0);
        }
        close(connections_file_descriptor);
    }


    return status;
}

int main(int argc, char const* argv[])
{
    //INIT
    pid_t process_id;
    /*
        Requirement: Open the connection to the system logger.
    */
    openlog(argv[0],0,LOG_USER);
   
    //NOTE: check to see if no args have been passed
    if (argv[1] == NULL)
    {
        start_server();
        exit(0);
    }

    process_id = fork();
    if (process_id < 0 )
    {
        fprintf(stderr, "ERROR: Failed to create daemon, error %d\n", errno);
        syslog(LOG_ERR, "ERROR: Failed to create daemon, process, error %d\n", errno);
        exit(1);
    }
    else if (process_id == 0)
    {
        pid_t child_process_id;
        child_process_id = getpid();
        printf("SUCCESS: The child process %d is still running.\n", child_process_id);
        syslog(LOG_DEBUG, "SUCCESS: The child process %d is still running.\n", child_process_id);
        if(setsid() == -1)
        {
            printf("ERROR: Failed to set the calling process as a leader...");
            syslog(LOG_ERR, "ERROR: Failed to set the calling process as a leader...%d\n", errno);
            exit(1);
        }
        if(chdir("/") == -1)
        {
            printf("ERROR: Failed to set the working directory to the root directory...");
            syslog(LOG_ERR, "ERROR: Failed to set the working directory to the root directory...%d\n", errno);
            exit(1);            
        }
        start_server();
        exit(0);
    }
    if(strcmp(argv[1], "-d") != 0)
    {
        printf("ERROR: Unexpected argument passed received...");
        syslog(LOG_ERR, "ERROR: Unexpected argument passed received.. Exiting: %d\n", errno);
        exit(1);
    }
    return 0;
}