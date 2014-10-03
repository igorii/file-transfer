#ifndef _MENU_
#define _MENU_

/* Menu action enum */
typedef enum {
    QUIT,
    LISTFILES,
    GETFILE,
    PUTFILE,
    RENAMEFILE
} menu_option;

menu_option handle_input (char *hostname, char *arg, int size);

#endif
