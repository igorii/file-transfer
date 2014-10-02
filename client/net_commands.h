#ifndef _NET_COMMANDS_
#define _NET_COMMANDS_

int request_file_list (int sock, int connection);
int request_put (int sock, char *filename);
int request_file (int sock, char *filename);
int setup (char *host, int *sock, int *conn);

#endif
