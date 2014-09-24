#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "helper.h"
#include "protocol.h"

#define DEBUG 0

#define SERVER_PORT 6005
#define MAX_LINE 246

// Menu action enum
typedef enum {QUIT, LISTFILES, GETFILE} menu_option;

/**
 * @return Success status
 */
int request_file_list (int sock, int connection) {
    char    *buffer;
    int      len;
    int      i;
    uint32_t num_items;

    send_byte(sock, DIR_LIST_CODE);
    len = recv_uint32(sock, &num_items);
    if (len <= 0) {
        return -1;
    }

    buffer = (char *) malloc (MAX_LINE);

    // Continuously handle chunks
    for(i = 0; i < num_items; ++i) {

        // Receive the next chunk
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

int request_file (int sock, char *filename) {
    uint32_t file_length;
    int recv_len;
    byte response;
    FILE *fp;

    printf("Opening %s\n", filename);
    fp = fopen(filename, "wb+");
    if (!fp) {
        return -1;
    }

    printf("Sending req code\n");

    // Send the file request code
    send_byte(sock, FILE_REQ_CODE);

    printf("Receiving req code\n");

    // Receive the incoming file acknowledgement
    recv_len = recv_byte(sock, &response);

    printf("Received length: %d\n", recv_len);
    if (recv_len <= 0) {
        return -1;
    }

    if (response != FILE_REQ_CODE) {
        return -1;
    }

    printf("Sending filename\n");

    // Send the file we are requesting
    send_line(sock, filename, strlen(filename));

    // Receive the incoming file length
    recv_len = recv_uint32(sock, &file_length);
    if (recv_len <= 0) {
        return -1;
    }

    if (file_length == 0) {
        fprintf(stderr, "[!!] Remote file does not exist or is empty\n");
        return -1;
    }

    printf("Receiving %d bytes\n", file_length);
    byte current_byte;
    unsigned int i;
    for (i = 0; i < file_length; ++i) {
        recv_len = recv_byte(sock, &current_byte);
        fwrite(&current_byte, 1, 1, fp);
    }

    return 0;
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
    const char *exit_cmd           = "exit\0";

    // Printf the prompt
    printf("file-serve(%s): ", hostname);

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
    char *host;
    int sock, conn, result;
    menu_option current_option;

    // Get server hostname
    if (argc == 2) {
        host = argv[1];
    } else {
        fprintf(stderr, "Usage: %s host\n", argv[0]);
        exit(1);
    }

    // Initialize the connection
    result = setup(host, &sock, &conn);
    if (result < 0) {
        exit(1);
    }

    // Request the desired action from the user
    char arg[256];
    for (;;) {
        current_option = handle_input(host, arg, sizeof(arg));

        switch (current_option) {

            case QUIT:
                close(sock);
                exit(1);
                break;

            case LISTFILES:
                request_file_list(sock, conn);
                break;

            case GETFILE:
                request_file(sock, arg);
                break;

            default:
                printf("nyi2\n");
                break;
        }
    }

    return 0;
}
