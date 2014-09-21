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

// Menu action enum
typedef enum {LISTFILES, GETFILE} menu_option;

int read_line(int sock, char *buffer, int size)
{
    int len = 0;
    char c;
    int ret;

    while (len < size)
    {
        ret = read(sock, &c, 1);
        if (ret <= 0) {
            buffer[len] = 0;
            return len;
        } else if (c == '\n') {
            buffer[len] = 0;
            return len;
        }

        if (c == '\0' && len == 0) {
            continue;
        }

        buffer[len++] = c;
    }

    return -1;
}

int request_file_list (int sock, int connection) {
    char *buffer;
    int len;
    const char *msg = "list_files\0";
    send(sock, msg, strlen(msg) + 1, 0);
    buffer = (char *) malloc (MAX_LINE);

    // Continuously handle chunks
    for(;;) {

        // Receive the next chunk
        len = read_line(sock, buffer, MAX_LINE - 1);

        // If we receive the ending token, break
        if (strcmp("*END-LISTING*", buffer) == 0) {
            break;
        }

        if (len <= 0) {
            break;
        }

        // Print the file received
        printf("%s\n", buffer);
    }

    free(buffer);
    return 0;
}

int request_file (int sock) {
    return 0;
}

/**
 * Print the menu and return the appropriate action
 */
menu_option handle_input () {
    char input;

    // Print the menu
    printf("Options:\n");
    printf("\t(1) List remote files\n");
    printf("\t(2) Retrieve remote files...\n");
    printf("What would you like to do? ");

    // Get the users option
    input = getchar();

    // Handle the option
    switch (input) {
        case '1':
            return LISTFILES;
        case '2':
            return GETFILE;
        default:
            printf("Invalid option\n");
            return handle_input();
    }
}

/**
 * Setup a socket and connection to the given host.
 * @param host The remote host to connect to
 * @param sock A pointer to the socket
 * @param conn A pointer to the connection handle
 * @return Success status
 */
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
    current_option = handle_input();
    printf("Menu option is %d\n", current_option);

    switch (current_option) {
        case LISTFILES:
            request_file_list(sock, conn);
            break;
        case GETFILE:
            printf("nyi\n");
            // TODO
            break;
    }

    return 0;
}
