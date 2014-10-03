#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

#include "helper.h"
#include "protocol.h"

#define MAX_PENDING 5

/**
 * Handles a request for an individual file
 * @param client_sock A socket connection
 * @return Success status
 */
int handle_file_request (int client_sock) {
    char *filename;
    filename = (char *) malloc (MAX_LINE);

    // Send the file request acknowledgement
    if (send_byte(client_sock, FILE_REQ_CODE) < 0) return -1;

    // Receive the file name
    if (recv_line(client_sock, filename, MAX_LINE) < 0) return -1;

    // Send the file
    if (send_file(client_sock, filename) < 0) return -1;

    free(filename);
    return 0;
}

int handle_put_file (int client_socket) {
    if (send_byte(client_socket, PUT_FILE_CODE) < 0) return -1;
    return recv_file(client_socket);
}

/**
 * Get the number of visible items in the directory
 * @param dir        The name of the directory to chec
 * @param num_items  Where the number of items will be placed
 * @return           Success status
 */
int get_num_items_in_dir(char *dir, int *num_items) {
    struct dirent *entry; // A single directory itemj
    DIR           *dirp;  // A pointer to a directory structure

    // Open the directory
    dirp = opendir(dir);
    if (dirp == NULL) {
        perror("opendir");
        return -1;
    }

    // Retrieve the number of items in the current directory
    *num_items = 0;
    while ( (entry = readdir(dirp)) ) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        ++*num_items;
    }

    // Close the directory
    closedir(dirp);
    return 0;
}

/**
 * Handles a request for a directory listing
 * @param client_sock   The client socket
 * @return              Success status
 */
int handle_file_list_request (int client_sock) {
    struct dirent *entry;     // A single directory item
    DIR           *dirp;      // Pointer to the dir structure
    int            num_items; // The number of items in the directory
    unsigned int   i;         // Looping variable

    // Get the number of visible items in the directory
    if (get_num_items_in_dir(".", &num_items) < 0) {
        return -1;
    }

    // Send the number of visible items
    if (send_uint32(client_sock, (uint32_t)num_items) < 0) return -1;

    // Open the directory before sending each item
    dirp = opendir(".");
    if (dirp == NULL) {
        perror("opendir");
        return -1;
    }

    // Send each item
    for (i = 0; i < num_items; ++i) {
        entry = readdir(dirp);

        // If the read failed, the item count changed somehow, so send an
        // empty line to keep the client in sync with the protocol
        if (!entry) {
            if (send_line(client_sock, "", 0) < 0) return -1;
        }

        // Invisible entries should not count towards the total
        if (entry->d_name[0] == '.') {
            --i;
            continue;
        }

        // Otherwise, send the entry to the client
        if (send_line(client_sock, entry->d_name, strlen(entry->d_name)) < 0) return -1;
    }

    // Cleanup the directory
    closedir(dirp);
    return 0;
}

/**
 * Setup the socket for connections
 * @param sock  A pointer to the socket
 * @param sin   A pointer to the sockaddr struct
 * @return      Success status
 */
int setup (int *sock, struct sockaddr_in *sin) {
    int bind_result;

    // Build address data structure
    memset(sin, 0, sizeof(*sin));
    sin->sin_family      = AF_INET;
    sin->sin_addr.s_addr = INADDR_ANY;
    sin->sin_port        = htons(SERVER_PORT);

    // Open the socket
    *sock = socket(PF_INET, SOCK_STREAM, 0);
    if (*sock < 0) {
        perror("socket");
        return -1;
    }

    // Bind the socket
    bind_result = bind(*sock, (struct sockaddr *) sin,
            sizeof(*sin));
    if (bind_result < 0) {
        perror("bind");
        return -2;
    }

    // Start listening for connections
    listen(*sock, MAX_PENDING);
    return 0;
}

/**
 * Accept a single connection on the given socket. The connection is
 * closed when finished.
 * @param sock The socket identifier to accept a connection on
 * @param sin  A pointer to the struct sockaddr_in
 * @return Success status
 */
int handle_connection (int sock, struct sockaddr_in *sin) {
    unsigned int accept_len;
    int          client_sock;
    byte         code;

    // Accept one connection
    accept_len  = sizeof(sin);
    client_sock = accept(sock, (struct sockaddr *) &sin,
            &accept_len);

    if (client_sock < 0) {
        perror("accept");
        return -1;
    }

    // Continuously handle chunks
    for (;;) {

        if (recv_byte(client_sock, &code) <= 0) {
            break;
        }

        // Handle the code appropriately
        switch (code) {
            case DIR_LIST_CODE:
                handle_file_list_request(client_sock);
                break;

            case FILE_REQ_CODE:
                handle_file_request(client_sock);
                break;

            case PUT_FILE_CODE:
                handle_put_file(client_sock);
                break;

            default:
                fprintf(stderr, "Unknown request [%d]\n", (int)code);
        }
    }

    close(client_sock);
    return 0;
}

int main (int argc, char *argv[]) {
    struct sockaddr_in sin;
    int sock, result;

    // Setup the socket
    result = setup(&sock, &sin);
    if (result < 0) {
        exit(1);
    }

    // Continuously handle connections
    for (;;) {

        // Handle one connection
        result = handle_connection(sock, &sin);
        if (result < 0) {
            exit (1);
        }
    }

    return 0;
}
