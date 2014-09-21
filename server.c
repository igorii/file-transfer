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

int setup (int *sock, struct sockaddr_in *sin) {
    int bindr, len;

    // build address data structure
    memset(sin, 0, sizeof(*sin));
    sin->sin_family      = AF_INET;
    sin->sin_addr.s_addr = INADDR_ANY;
    sin->sin_port        = htons(SERVER_PORT);

    // setup passive open
    *sock = socket(PF_INET, SOCK_STREAM, 0);
    if (*sock < 0) {
        perror("socket");
        return -1;
    }

    bindr = bind(*sock, (struct sockaddr *) sin, sizeof(*sin));
    if (bindr < 0) {
        perror("bind");
        return -2;
    }

    listen(*sock, MAX_PENDING);
    return 0;
}

int handleconnectionection (int sock, struct sockaddr_in *sin) {
    int accept_len;
    int len, connection;
    char buffer[MAX_LINE];

#if DEBUG > 0
    printf("Accepting on %d\n", sock);
#endif

    // Accept one connectionection
    accept_len = sizeof(sin);
    connection = accept(sock, (struct sockaddr *) &sin, &accept_len);
    if (connection < 0) {
        perror("accept");
        return -1;
    }

#if DEBUG > 0
    printf("Receiving...\n");
#endif

    // Wait for connectionection, then receive and print text
    while (len = recv(connection, buffer, sizeof(buffer), 0)) {
        printf("Received %d\n", len);
        fputs(buffer, stdout);
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
        result = handleconnectionection(sock, &sin);
        if (result < 0) {
            exit (1);
        }
    }

    return 0;
}
