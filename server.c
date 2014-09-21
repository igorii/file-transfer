#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* Directory operations */
#include <dirent.h>

#define DEBUG 0

#define SERVER_PORT 6005
#define MAX_PENDING 5
#define MAX_LINE 256

int handle_file_request () {
    printf("Handling file retrieve request\n");
    return 0;
}
int handle_file_list_request (int sock, int client_sock) {
    struct dirent *entry;
    DIR *dirp;
    const char *final_message = "*END-LISTING*\n";
    printf("Handling file list request\n");

    dirp = opendir(".");
    if (dirp == NULL) {
        perror("opendir");
        return -1;
    }

    char buffer[MAX_LINE];

    while((entry = readdir(dirp))) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        snprintf(buffer, MAX_LINE, "%s\n", entry->d_name);
        send(client_sock, buffer, strlen(buffer) + 1, 0);
    }

    snprintf(buffer, MAX_LINE, "%s", final_message);
    send(client_sock, buffer, strlen(buffer) + 1, 0);
    closedir(dirp);
    return 0;
}

/**
 * Setup the socket for connections
 * @param sock A pointer to the socket
 * @param sin  A pointer to the sockaddr struct
 * @return Success status
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

    bind_result = bind(*sock, (struct sockaddr *) sin, sizeof(*sin));
    if (bind_result < 0) {
        perror("bind");
        return -2;
    }

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
    int len, client_sock;
    char buffer[MAX_LINE];

    // Accept one connection
    accept_len = sizeof(sin);
    client_sock = accept(sock, (struct sockaddr *) &sin, &accept_len);
    if (client_sock < 0) {
        perror("accept");
        return -1;
    }

    // Continuously handle chunks
    for(;;) {

        // Receive the next chunk
        len = recv(client_sock, buffer, sizeof(buffer), 0);
        printf("Received %s [%d]\n", buffer, len);
        if (len <= 0) {
            break;
        }

        if (strcmp("list_files", buffer) == 0) {
            handle_file_list_request(sock, client_sock);
        } else {
            printf("Unknown request\n");
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
