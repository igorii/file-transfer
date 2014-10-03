#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "helper.h"
#include "protocol.h"
#include "menu.h"
#include "net_commands.h"

int main (int argc, char* argv[]) {
    menu_option current_option;   // Menu action
    char        arg[256];         // Menu action argument
    char       *host;             // Hostname to connect to
    int         sock,             // Socket to host
                conn,             // Connection to the host
                result;           // Storage for the success
                                  //     status of commands

    // Get server hostname
    if (argc == 2) {
        host = argv[1];
    } else {
        fprintf(stderr, "Usage: %s host\n", argv[0]);
        exit(1);
    }

    // Initialize the connection
    printf("Connecting...");
    fflush(stdout);

    result = setup(host, &sock, &conn);
    if (result < 0) {
        fprintf(stderr, "\nCould not connect to %s:%d\n", host, SERVER_PORT);
        exit(1);
    }

    printf("\rConnected to %s:%d\n", host, SERVER_PORT);

    // Request the desired action from the user
    for (;;) {
        current_option = handle_input(host, arg, sizeof(arg));

        switch (current_option) {

            case QUIT:
                close(sock);
                exit(1);
                break;

            // TODO - handle errors from commands (when return is < 0)

            case LISTFILES:
                request_file_list(sock);
                break;

            case GETFILE:
                request_file(sock, arg);
                break;

            case PUTFILE:
                request_put(sock, arg);
                break;

            case RENAMEFILE:
                request_rename(sock, arg);
                break;

            default:
                fprintf(stderr, "[!!] Unknown command\n");
                break;
        }
    }

    return 0;
}
