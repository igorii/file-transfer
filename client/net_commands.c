#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "helper.h"
#include "net_commands.h"
#include "protocol.h"

/**
 * Request that the server rename a remote file
 * @param sock     A socket descriptor
 * @param filename The name of the file to rename
 * @return         Success status
 */
int request_rename (int sock, char *filename) {
    char new_filename[MAX_LINE];  // Input for the new filename
    byte response;                // The acknowledgement from the other

    // Get the new filename
    printf("  to: ");
    fgets(new_filename, MAX_LINE, stdin);
    new_filename[strlen(new_filename) - 1] = '\0';

    // Send the file request code
    if (send_byte(sock, FILE_RENAME_CODE) < 0)
        return -1;

    // Receive the incoming file acknowledgement
    if (recv_byte(sock, &response) <= 0)
        return -1;

    if (response != FILE_RENAME_CODE)
        return -1;

    // Send the filename we are renaming
    if (send_line(sock, filename, strlen(filename)) < 0)
        return -1;

    // Send the new filename
    if (send_line(sock, new_filename, strlen(new_filename)) < 0)
        return -1;

    return 0;
}

/**
 * Request a directory listing from the remote server
 * @param sock A socket descriptor
 * @return     Success status
 */
int request_file_list (int sock) {
    char    *buffer;     // A buffer to hold each entry
    int      len;        // Storage for the number of bytes received
    uint32_t num_items;  // The number of items in the remote directory
    int      i;          // Looping variable

    // Send the operation code
    if (send_byte(sock, DIR_LIST_CODE) < 0)
        return -1;

    // Store the number of files being received
    if (recv_uint32(sock, &num_items) <= 0)
        return -1;

    // Create a place to store the currently received file
    buffer = (char *) malloc (MAX_LINE);

    // Receive each file
    for(i = 0; i < num_items; ++i) {
        if (recv_line(sock, buffer, MAX_LINE - 1) <= 0)
            break;

        printf("- %s\n", buffer); // Print the file received
    }

    free(buffer);                 // Free the entry buffer
    return 0;
}

/**
 * Request the server to create a remote copy of a local file.
 * @param sock     A socket descriptor
 * @param filename The name of the local file to send
 * @return         Success status
 */
int request_put (int sock, char *filename) {
    byte response; // The acknowledgement from the other

    if (send_byte(sock, PUT_FILE_CODE) < 0) return -1;

    // Receive the incoming file acknowledgement
    if (recv_byte(sock, &response) <= 0)
        return -1;

    // Ensure the Server is in the correct state
    if (response != PUT_FILE_CODE)
        return -1;

    // Send the file
    return send_file(sock, filename);
}


/**
 * Request a file to be written locally in the current directory
 * @param sock     A socket connection
 * @param filename The name of the file being requested
 * @return         Success status
 */
int request_file (int sock, char *filename) {
    byte response; // The acknowledgement from the other

    // Send the file request code
    if (send_byte(sock, FILE_REQ_CODE) < 0)
        return -1;

    // Receive the incoming file acknowledgement
    if (recv_byte(sock, &response) <= 0)
        return -1;

    if (response != FILE_REQ_CODE)
        return -1;

    // Send the filename we are requesting
    if (send_line(sock, filename, strlen(filename)) < 0)
        return -1;

    // Receive the file
    return recv_file(sock);
}

/**
 * Setup a socket and connection to the given host.
 * @param host    The remote host to connect to
 * @param portnum The port of the host
 * @param sock    A pointer to the socket
 * @param conn    A pointer to the connection handle
 * @return        Success status
 */
int setup (char *host, unsigned short portnum, int *sock, int *conn) {
    struct sockaddr_in  sin;
    struct hostent     *hp;

    // Translate hostname
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "unknown host: %s\n", host);
        return -1;
    }

    // Build address data structure and create socket
    memset(&sin, 0, sizeof(sin));                            // Initialize to 0
    sin.sin_family = AF_INET;                                // Internet addr family
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);  // Set server addr
    sin.sin_port = htons(portnum);                           // Set the server port number
    *sock        = socket(PF_INET, SOCK_STREAM, 0);          // Create the socket
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
