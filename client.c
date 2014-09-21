#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 6005
#define MAX_LINE 246

int main (int argc, char* argv[]) {
    FILE *fd;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buffer[MAX_LINE];

    int s, conn, len;

    // get server hostname
    if (argc == 2) {
        host = argv[1];
    } else {
        fprintf(stderr, "Usage: simplex-talk host\n");
        exit(1);
    }

    // translate hostname
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
        exit(1);
    }

    // build address data structure
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);

    // active open
    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }

    conn = connect(s, (struct sockaddr *)&sin, sizeof(sin));
    if (conn < 0) {
        perror("simplex-talk: connect");
        close(s);
        exit(1);
    }

    printf("Connected...\n");

    // main loop: get and send lines of text
    while (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[MAX_LINE-1] = '\0';
        len = strlen(buffer) + 1;
        printf("Sending %s\n", buffer);
        send(s, buffer, len, 0);
    }

    return 0;
}
