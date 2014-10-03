#include <stdio.h>
#include <string.h>
#include "menu.h"

#define BUFSIZE 1024

/**
 * Handle input that contains an argument. Store the argument in `arg`
 * @param option  The menu option to return
 * @param input   The raw input from the user
 * @param cmd     The cmd itself
 * @param cmd_len The length of the string before the argument
 * @param arg     Out param to return the argument
 * @param use     A string depicting the use of the command
 * @return        The menu option or -1 on error
 */
int handle_cmd_with_arg(menu_option option,
        char *input, const char *cmd,
        int cmd_len, char *arg, char *use) {

    // The input for get must be longer than the length of
    // "get " since it requires a filename argument
    if (strlen(input) > cmd_len) {
        memcpy(arg, input + cmd_len, strlen(input) - cmd_len);
        arg[strlen(input) - (1 + cmd_len)] = '\0';
        return option;
    } else {
        fprintf(stderr, "\tUse: '%s'\n", use);
        return -1;
    }
}

/**
 * Print the menu and return the appropriate action
 * @param hostname  The name of the host server
 * @param arg       A character array for an optional command argument
 * @param size      The size of `arg`
 * @return          The menu option specified by the user
 */
menu_option handle_input (char *hostname, char *arg, int size) {
    char input[BUFSIZE]; // Input buffer

    // Commands
    const char *file_list_cmd      = "ls\0";
    const char *file_retrieval_cmd = "get\0";
    const char *file_send_cmd      = "put\0";
    const char *file_rename_cmd    = "rename\0";
    const char *exit_cmd           = "exit\0";

    // Length of command before argument
    const int file_list_cmd_len      = strlen(file_list_cmd);
    const int file_retrieval_cmd_len = 1 + strlen(file_retrieval_cmd);
    const int file_send_cmd_len      = 1 + strlen(file_send_cmd);
    const int file_rename_cmd_len    = 1 + strlen(file_rename_cmd);

    // Print the prompt
    printf("file-serve(%s): ", hostname);
    fflush(stdout);

    // Get the user input
    fgets(input, sizeof(input), stdin);

    // Handle directory listing requests
    if (strncmp(file_list_cmd, input, strlen(file_list_cmd)) == 0) {
        return LISTFILES;

    // Handle file retrieval requests
    } else if (strncmp(file_retrieval_cmd, input, strlen(file_retrieval_cmd)) == 0) {
        return handle_cmd_with_arg(GETFILE, input,
                file_retrieval_cmd, file_retrieval_cmd_len,
                arg, "get <filename>");

    // Handle file send requests
    } else if (strncmp(file_send_cmd, input, strlen(file_send_cmd)) == 0) {
        return handle_cmd_with_arg(PUTFILE, input,
                file_send_cmd, file_send_cmd_len,
                arg, "put <filename>");

    // Handle file rename requests
    } else if (strncmp(file_rename_cmd, input, strlen(file_rename_cmd)) == 0) {
        return handle_cmd_with_arg(PUTFILE, input,
                file_rename_cmd, file_rename_cmd_len,
                arg, "rename <filename>");

    // Handle exit requests
    } else if (strncmp(exit_cmd, input, strlen(exit_cmd)) == 0) {
        return QUIT;
    }

    // If none of the above was handled, just try again
    return handle_input(hostname, arg, size);
}
