#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "helper.h"
#include "net_commands.h"
#include "protocol.h"

/**
 * @return Success status
 */
int request_rename (int sock, char *filename) {
    char new_filename[MAX_LINE];  // Input for the new filename
    byte response;                // The acknowledgement from the other

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
 * @return Success status
 */
int request_file_list (int sock) {
    char    *buffer;
    int      len;
    int      i;
    uint32_t num_items;

    // Send the operation code
    if (send_byte(sock, DIR_LIST_CODE) < 0) return -1;

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

    if (send_byte(sock, PUT_FILE_CODE) < 0) return -1;

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
    if (send_byte(sock, FILE_REQ_CODE) < 0) return -1;

    // Receive the incoming file acknowledgement
    if (recv_byte(sock, &response) <= 0)
        return -1;

    if (response != FILE_REQ_CODE)
        return -1;

    // Send the filename we are requesting
    if (send_line(sock, filename, strlen(filename)) < 0) return -1;

    // Receive the file
    return recv_file(sock);
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
