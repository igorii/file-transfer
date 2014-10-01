#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "helper.h"
#include "protocol.h"

/* Menu action enum */
typedef enum {QUIT, LISTFILES, GETFILE, PUTFILE} menu_option;

/**
 * @return Success status
 */
int request_file_list (int sock, int connection) {
    char    *buffer;
    int      len;
    int      i;
    uint32_t num_items;

    // Send the operation code
    send_byte(sock, DIR_LIST_CODE);

    // Store the number of files being received
    len = recv_uint32(sock, &num_items);
    if (len <= 0) {
        return -1;
    }

    // Create a place to store the currently received file
    buffer = (char *) malloc (MAX_LINE);

    // Receive each file
    for(i = 0; i < num_items; ++i) {
        len = recv_line(sock, buffer, MAX_LINE - 1);
        if (len <= 0) {
            break;
        }

        // Print the file received
        printf("- %s\n", buffer);
    }

    free(buffer);
    return 0;
}

int request_put (int sock, char *filename) {
    byte response; // The acknowledgement from the other

    send_byte(sock, PUT_FILE_CODE);

    // Receive the incoming file acknowledgement
    if (recv_byte(sock, &response) <= 0)
        return -1;

    if (response != PUT_FILE_CODE)
        return -1;

    return send_file(sock, filename);
}


/**
 * Request a file to be written locally in the current directory
 * @param sock     A socket connection
 * @param filename The name of the file being requested
 * @return Success status
 */
int request_file (int sock, char *filename) {
    byte response; // The acknowledgement from the other

    // Send the file request code
    send_byte(sock, FILE_REQ_CODE);

    // Receive the incoming file acknowledgement
    if (recv_byte(sock, &response) <= 0)
        return -1;

    if (response != FILE_REQ_CODE)
        return -1;

    // Send the filename we are requesting
    send_line(sock, filename, strlen(filename));

    // Receive the file
    return recv_file(sock);
}


/**
 * Print the menu and return the appropriate action
 * @param hostname  The name of the host server
 * @param arg       A character array for an optional command argument
 * @param size      The size of `arg`
 * @return          The menu option specified by the user
 */
menu_option handle_input (char *hostname, char *arg, int size) {
    char input[256]; // Input buffer

    // Commands
    const char *file_list_cmd      = "ls\0";
    const char *file_retrieval_cmd = "get\0";
    const char *file_send_cmd      = "put\0";
    const char *exit_cmd           = "exit\0";

    // Printf the prompt
    printf("file-serve(%s): ", hostname);
    fflush(stdout);

    // Get the user input
    fgets(input, sizeof(input), stdin);

    // Handle directory listing requests
    if (strncmp(file_list_cmd, input, strlen(file_list_cmd)) == 0) {
        return LISTFILES;
    }

    // Handle file retrieval requests
    else if (strncmp(file_retrieval_cmd, input,
                strlen(file_retrieval_cmd)) == 0) {

        // The input for get must be longer than the length of
        // "get " since it requires a filename argument
        //
        // TODO - figure this out
        if (strlen(input) > 4) {
            memcpy(arg, input + 4, strlen(input) - 4);
            arg[strlen(input) - 5] = '\0';
            return GETFILE;
        } else {
            fprintf(stderr, "\tUse: 'get <filename>'\n");
        }

    // Handle file send requests
    } else if (strncmp(file_send_cmd, input,
                strlen(file_send_cmd)) == 0) {

        // TODO - figure this out
        if (strlen(input) > 4) {
            memcpy(arg, input + 4, strlen(input) - 4);
            arg[strlen(input) - 5] = '\0';
            return PUTFILE;
        } else {
            fprintf(stderr, "\tUse: 'put <filename>'\n");
        }
    } else if (strncmp(exit_cmd, input, strlen(exit_cmd)) == 0) {
        return QUIT;
    }

    // If none of the above was handled, just try again
    return handle_input(hostname, arg, size);
}

/**
 * Setup a socket and connection to the given host.
 * @param host  The remote host to connect to
 * @param sock  A pointer to the socket
 * @param conn  A pointer to the connection handle
 * @return      Success status
 */
int setup (char *host, int *sock, int *conn) {
    struct sockaddr_in  sin;
    struct hostent     *hp;

    // Translate hostname
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "unknown host: %s\n", host);
        return -1;
    }

    // Build address data structure
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);

    // Open the socket
    *sock = socket(PF_INET, SOCK_STREAM, 0);
    if (*sock < 0) {
        perror("socket");
        return -2;
    }

    // Create the connection
    *conn = connect(*sock, (struct sockaddr *)&sin, sizeof(sin));
    if (*conn < 0) {
        perror("connect");
        close(*sock);
        return -3;
    }

    return 0;
}

int main (int argc, char* argv[]) {
    menu_option current_option;
    char        arg[256];
    char       *host;
    int         sock,
                conn,
                result;

    // Get server hostname
    if (argc == 2) {
        host = argv[1];
    } else {
        fprintf(stderr, "Usage: %s host\n", argv[0]);
        exit(1);
    }

    // Initialize the connection
    printf("Connecting...");
    fflush(stdout);

    result = setup(host, &sock, &conn);
    if (result < 0) {
        fprintf(stderr, "\nCould not connect to %s:%d\n", host, SERVER_PORT);
        exit(1);
    }

    printf("\rConnected to %s:%d\n", host, SERVER_PORT);

    // Request the desired action from the user
    for (;;) {
        current_option = handle_input(host, arg, sizeof(arg));

        switch (current_option) {

            case QUIT:
                close(sock);
                exit(1);
                break;

            // TODO - handle errors from commands (when return is < 0)

            case LISTFILES:
                request_file_list(sock, conn);
                break;

            case GETFILE:
                request_file(sock, arg);
                break;

            case PUTFILE:
                request_put(sock, arg);
                break;

            default:
                fprintf(stderr, "[!!] Unknown command\n");
                break;
        }
    }

    return 0;
}
