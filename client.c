#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEBUG 0

#define SERVER_PORT 6005
#define MAX_LINE 246

int setup (char *host, int *sock, int *conn) {
    struct sockaddr_in sin;
    struct hostent *hp;

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

    // active open
    *sock = socket(PF_INET, SOCK_STREAM, 0);
    if (*sock < 0) {
        perror("socket");
        return -2;
    }

    *conn = connect(*sock, (struct sockaddr *)&sin, sizeof(sin));
    if (*conn < 0) {
        perror("connect");
        close(*sock);
        return -3;
    }

#if DEBUG > 0
    printf("Connected...\n");
#endif

    return 0;
}

int main (int argc, char* argv[]) {
    char *host;
    char buffer[MAX_LINE];
    int sock, conn, len;
    int result;

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

    // main loop: get and send lines of text
    while (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[MAX_LINE-1] = '\0';
        len = strlen(buffer) + 1;
        printf("Sending %s\n", buffer);
        send(sock, buffer, len, 0);
    }

    return 0;
}
