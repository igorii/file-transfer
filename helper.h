#ifndef _HELPER_
#define _HELPER_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

typedef unsigned char byte;

int send_byte(int sock, byte value);
int send_uint32(int sock, uint32_t value);
int send_line(int sock, char *buffer, int size);

int recv_byte(int sock, byte *value);
int recv_uint32(int sock, uint32_t *value);
int recv_line(int sock, char *buffer, int size);

#endif
