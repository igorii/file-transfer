#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "helper.h"


/* ************** */
/* Byte Send/Recv */
/* ************** */

int send_byte (int sock, byte value) {
    return send(sock, &value, 1, 0);
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
    return send(sock, &to_send, sizeof(uint32_t), 0);
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
    return send(sock, to_send, sizeof(to_send), 0);
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

/* ************** */
/* File Send/Recv */
/* ************** */

int send_file (int sock, char *filename) {
    FILE         *fp;
    byte          current_byte;
    int           read_len;
    unsigned int  file_length;
    unsigned int  i;

    if (send_line(sock, filename, strlen(filename)) < 0) return -1;

    // Open the file to determine the number of bytes that
    // need to be sent
    fp = fopen(filename, "rb");
    if (!fp) {
        if (send_uint32(sock, 0) < 0) return -1;
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    file_length = ftell(fp);

#if DEBUG
    printf("Sending %s | %d bytes | ... ", filename, file_length);
    fflush(stdout);
#endif

    // Send the file size to the client
    if (send_uint32(sock, file_length) < 0) return -1;

    // Send each byte to the client starting from the start
    rewind(fp);
    for (i = 0; i < file_length; ++i) {
        read_len = fread(&current_byte, 1, 1, fp);
        if (read_len <= 0) {

            // send 0 to keep the client in sync
            if (send_byte(sock, 0) < 0) return -1;
            continue;
        }

#if DEBUG
        printf("\rSending %s | %d bytes | %d%%... ", filename, file_length,
                (int) (100 * floor((i + 1) / file_length)));
        fflush(stdout);
#endif

        if (send_byte(sock, current_byte) < 0) return -1;
    }

    // Close the file
    fclose(fp);

#if DEBUG
    printf("Done.\n");
#endif
    return 0;
}

int recv_file (int sock) {
    int          recv_len;      // The number of bytes received from a recv
    char        *filename;
    FILE        *fp;            // The local file to write
    uint32_t     file_length;   // The length of the incoming file in bytes
    unsigned int i;
    byte         current_byte;  // The current byte received

    filename = (char *) malloc (MAX_LINE);

    // receive filename
    if (recv_line(sock, filename, MAX_LINE) <= 0) {
        return -1;
    }

    // Receive the incoming file length
    if (recv_uint32(sock, &file_length) <= 0)
        return -1;

    // Ensure that the file exists
    if (file_length == 0) {
        fprintf(stderr, "[!!] Remote file does not exist or is empty\n");
        return -1;
    }

#if DEBUG
    printf("Receiving %s | %d bytes | ... ", filename, file_length);
    fflush(stdout);
#endif

    // If it does exist, open a local copy as binary
    fp = fopen(filename, "wb+");
    if (!fp) {
        return -1;
    }

    // Loop over each byte and write the received byte to the local file
    for (i = 0; i < file_length; ++i) {
        recv_len = recv_byte(sock, &current_byte);
        if (recv_len <= 0) {
            fclose(fp);
            return -1;
        }

#if DEBUG
        printf("\rReceiving %s | %d bytes | %d%%... ", filename, file_length,
                (int) (100 * floor((i + 1) / file_length)));
        fflush(stdout);
#endif
        fwrite(&current_byte, 1, 1, fp);
    }

    // Close the local file when finished
    fclose(fp);
    free(filename);

#if DEBUG
    printf("Done.\n");
#endif
    return 0;
}
