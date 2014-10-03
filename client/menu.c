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
    const char *file_rename_cmd    = "rename\0";
    const char *exit_cmd           = "exit\0";

    const int file_list_cmd_len      = strlen(file_list_cmd);
    const int file_retrieval_cmd_len = 1 + strlen(file_retrieval_cmd);
    const int file_send_cmd_len      = 1 + strlen(file_send_cmd);
    const int file_rename_cmd_len    = 1 + strlen(file_rename_cmd);

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
        if (strlen(input) > file_retrieval_cmd_len) {
            memcpy(arg,
                    input + file_retrieval_cmd_len,
                    strlen(input) - file_retrieval_cmd_len);
            arg[strlen(input) - (1 + file_retrieval_cmd_len)] = '\0';
            return GETFILE;
        } else {
            fprintf(stderr, "\tUse: 'get <filename>'\n");
        }

    // Handle file send requests
    } else if (strncmp(file_send_cmd, input,
                strlen(file_send_cmd)) == 0) {

        if (strlen(input) > file_send_cmd_len) {
            memcpy(arg,
                    input + file_send_cmd_len,
                    strlen(input) - file_send_cmd_len);
            arg[strlen(input) - (1 + file_send_cmd_len)] = '\0';
            return PUTFILE;
        } else {
            fprintf(stderr, "\tUse: 'put <filename>'\n");
        }
    }

    // Handle file rename requests
    else if (strncmp(file_rename_cmd, input,
                strlen(file_rename_cmd)) == 0) {
        if (strlen(input) > file_rename_cmd_len) {
            memcpy(arg,
                    input + file_rename_cmd_len,
                    strlen(input) - file_rename_cmd_len);
            arg[strlen(input) - (1 + file_rename_cmd_len)] = '\0';
            return RENAMEFILE;
        } else {
            fprintf(stderr, "\tUse: 'rename <filename>'\n");
        }
    } else if (strncmp(exit_cmd, input, strlen(exit_cmd)) == 0) {
        return QUIT;
    }

    // If none of the above was handled, just try again
    return handle_input(hostname, arg, size);
}
