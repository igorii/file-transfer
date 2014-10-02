#include <stdio.h>
#include <string.h>
#include "menu.h"

/**
 * Print the menu and return the appropriate action
 * @param hostname  The name of the host server
 * @param arg       A character array for an optional command argument
 * @param size      The size of `arg`
 * @return          The menu option specified by the user
 */
menu_option handle_input (char *hostname, char *arg, int size) {
    char input[256]; // Input buffer

    // Commands
    const char *file_list_cmd      = "ls\0";
    const char *file_retrieval_cmd = "get\0";
    const char *file_send_cmd      = "put\0";
    const char *exit_cmd           = "exit\0";

    // Printf the prompt
    printf("file-serve(%s): ", hostname);
    fflush(stdout);

    // Get the user input
    fgets(input, sizeof(input), stdin);

    // Handle directory listing requests
    if (strncmp(file_list_cmd, input, strlen(file_list_cmd)) == 0) {
        return LISTFILES;
    }

    // Handle file retrieval requests
    else if (strncmp(file_retrieval_cmd, input,
                strlen(file_retrieval_cmd)) == 0) {

        // The input for get must be longer than the length of
        // "get " since it requires a filename argument
        //
        // TODO - figure this out
        if (strlen(input) > 4) {
            memcpy(arg, input + 4, strlen(input) - 4);
            arg[strlen(input) - 5] = '\0';
            return GETFILE;
        } else {
            fprintf(stderr, "\tUse: 'get <filename>'\n");
        }

    // Handle file send requests
    } else if (strncmp(file_send_cmd, input,
                strlen(file_send_cmd)) == 0) {

        // TODO - figure this out
        if (strlen(input) > 4) {
            memcpy(arg, input + 4, strlen(input) - 4);
            arg[strlen(input) - 5] = '\0';
            return PUTFILE;
        } else {
            fprintf(stderr, "\tUse: 'put <filename>'\n");
        }
    } else if (strncmp(exit_cmd, input, strlen(exit_cmd)) == 0) {
        return QUIT;
    }

    // If none of the above was handled, just try again
    return handle_input(hostname, arg, size);
}
