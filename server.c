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
    int len, connection;
    char buffer[MAX_LINE];

    // Accept one connectionection
    accept_len = sizeof(sin);
    connection = accept(sock, (struct sockaddr *) &sin, &accept_len);
    if (connection < 0) {
        perror("accept");
        return -1;
    }

    // Wait for connection, then receive and print text
    len = recv(connection, buffer, sizeof(buffer), 0);
    while (len) {

        // Handle the chunk
        fputs(buffer, stdout);

        // Receive the next chun
        len = recv(connection, buffer, sizeof(buffer), 0);
    }

    close(connection);
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

    // Continuously handle connectionections
    for (;;) {

        // Handle one connectionection
        result = handle_connection(sock, &sin);
        if (result < 0) {
            exit (1);
        }
    }

    return 0;
}
