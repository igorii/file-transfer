
#include "helper.h"

#include <stdio.h>

/* ************** */
/* Byte Send/Recv */
/* ************** */

int send_byte (int sock, byte value) {
    send(sock, &value, 1, 0);
    return 0;
}

/**
 * @return Success status
 */
int recv_byte (int sock, byte *value) {
    return recv(sock, value, 1, 0);
}

/* ****************** */
/* UINT32_T Send/Recv */
/* ****************** */

/**
 * @return Success status
 */
int send_uint32(int sock, uint32_t value) {
    uint32_t to_send = htonl(value);
    send(sock, &to_send, sizeof(uint32_t), 0);
    return 0;
}

/**
 * @return Success status
 */
int recv_uint32 (int sock, uint32_t *value) {
    int      len;
    uint32_t raw;
    len    = recv(sock, &raw, 4, 0);
    *value = ntohl(raw);
    return len;
}

/* ************** */
/* Line Send/Recv */
/* ************** */

/**
 * Take an unterminated array of characters and terminate
 * with a newline before sending
 */
int send_line (int sock, char *buffer, int size) {
    char to_send[size + 1];
    memcpy(to_send, buffer, size);
    to_send[size] = '\n';
    send(sock, to_send, sizeof(to_send), 0);
    return 0;
}

/**
 * @return Number of bytes recv
 */
int recv_line(int sock, char *buffer, int size)
{
    int curr, len;
    char c;
    curr = 0;

    while (curr < size)
    {
        len = recv(sock, &c, 1, 0);
        if (len <= 0) {
            buffer[curr] = 0;
            return curr;
        } else if (c == '\n') {
            buffer[curr] = 0;
            return curr;
        }

        buffer[curr++] = c;
    }

    return -1;
}

