#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "helper.h"


/* ************** */
/* Byte Send/Recv */
/* ************** */

/**
 * Send a single byte
 * @param sock  A socket descriptor
 * @param value The value of the byte to send
 * @return      The number of bytes sent or -1 on error
 */
int send_byte (int sock, byte value) {
    return send(sock, &value, 1, 0);
}

/**
 * Receive a single byte
 * @param sock  A socket descriptor
 * @param value The value of the byte to receive (out param)
 * @return      The number of bytes received or -1 on error
 */
int recv_byte (int sock, byte *value) {
    return recv(sock, value, 1, 0);
}

/* ****************** */
/* UINT32_T Send/Recv */
/* ****************** */

/**
 * Send a single unsigned 32 bit value
 * @param sock  A socket descriptor
 * @param value The value of the byte to send
 * @return      The number of bytes sent or -1 on error
 */
int send_uint32(int sock, uint32_t value) {
    uint32_t to_send = htonl(value);
    return send(sock, &to_send, sizeof(uint32_t), 0);
}

/**
 * Receive a single unsigned 32 bit value
 * @param sock  A socket descriptor
 * @param value The value of the byte to receive (out param)
 * @return The number of bytes received or -1 on error
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
 * @param sock   A socket descriptor
 * @param buffer The line being recevied (out param)
 * @param size   The string length of the line being sent
 * @return       The number of bytes sent or -1 on error
 */
int send_line (int sock, char *buffer, int size) {
    char to_send[size + 1];
    memcpy(to_send, buffer, size);
    to_send[size] = '\n';
    return send(sock, to_send, sizeof(to_send), 0);
}

/**
 * Receive a line terminated array of characters
 * @param sock    A socket descriptor
 * @param buffer  The receiving buffer (out param)
 * @param size    The max size of the receiving buffer
 * @return        Number of bytes received or -1 on error
 */
int recv_line(int sock, char *buffer, int size) {
    int curr,  // Current index
        len;   // Number of received bytes
    char c;    // Received character

    curr = 0; // Initialize the current index

    while (curr < size) {

        len = recv(sock, &c, 1, 0);  // Receive one character
        if (len <= 0) {
            buffer[curr] = 0;
            return curr;

        // If receiving a newline, return the string
        } else if (c == '\n') {
            buffer[curr] = 0;
            return curr;
        }

        // Otherwise add the character to the buffer
        buffer[curr++] = c;
    }

    // The input buffer is full
    // Consume bytes until a newline is received, but signal an error
    for (;;) {

        len = recv(sock, &c, 1, 0);  // Receive one character
        if (len <= 0) {
            buffer[curr] = 0;
            return -1;

        // If receiving a newline, return the string
        } else if (c == '\n') {
            buffer[curr] = 0;
            return -1;
        }
    }

    return -1;
}

/* ************** */
/* File Send/Recv */
/* ************** */

/**
 * Send a single file
 * @param sock A socket descriptor
 * @param filename The name of the local file being sent
 * @return Success status
 */
int send_file (int sock, char *filename) {
    FILE         *fp;            // File pointer to the local file
    byte          current_byte;  // The current byte of the file
    int           read_len;      // The number of items read from the file
    unsigned int  file_length;   // The length of the file
    unsigned int  i;             // Looping variable

    // Send the file name
    if (send_line(sock, filename, strlen(filename)) < 0)
        return -1;

    // Open the file to determine the number of bytes that
    // need to be sent
    fp = fopen(filename, "rb");
    if (!fp) {
        if (send_uint32(sock, 0) < 0) return -1; // Send 0 to signal an error
        return -1;
    }

    fseek(fp, 0L, SEEK_END); // Determine the file length
    file_length = ftell(fp);

#if VERBOSE
    printf("Sending %s | %d bytes | ... ", filename, file_length);
    fflush(stdout);
#endif

    // Send the file size to the client
    if (send_uint32(sock, file_length) < 0) {
        fclose(fp);
        return -1;
    }

    // Send each byte to the client starting from the start
    rewind(fp);
    for (i = 0; i < file_length; ++i) {
        read_len = fread(&current_byte, 1, 1, fp);
        if (read_len <= 0) {

            // Send 0 to keep the client in sync
            if (send_byte(sock, 0) < 0) {
                fclose(fp);
                return -1;
            }

            continue;
        }

        // Otherwise send the byte
        if (send_byte(sock, current_byte) < 0) {
            fclose(fp);
            return -1;
        }

#if VERBOSE
        printf("\rSending %s | %d bytes | %d%%... ", filename, file_length,
                (int) (100 * floor((i + 1) / file_length)));
        fflush(stdout);
#endif
    }

    fclose(fp); // Close the file

#if VERBOSE
    printf("Done.\n");
#endif

    return 0;
}

/**
 * Receive a single file
 * @param sock A socket descriptor
 * @return     Success status
 */
int recv_file (int sock) {
    int          recv_len;      // The number of bytes received from a recv
    char        *filename;      // The name of the file being received
    FILE        *fp;            // The local file to write
    uint32_t     file_length;   // The length of the incoming file in bytes
    byte         current_byte;  // The current byte received
    unsigned int i;             // Looping variable

    filename = (char *) malloc (MAX_LINE); // Initialize the name buffer

    // Receive filename
    if (recv_line(sock, filename, MAX_LINE) <= 0) {
        free(filename);
        return -1;
    }

    // Receive the incoming file length
    if (recv_uint32(sock, &file_length) <= 0) {
        free(filename);
        return -1;
    }

    // Ensure that the file exists
    if (file_length == 0) {
        fprintf(stderr, "[!!] Remote file does not exist or is empty\n");
        free(filename);
        return -1;
    }

#if VERBOSE
    printf("Receiving %s | %d bytes | ... ", filename, file_length);
    fflush(stdout);
#endif

    // If it does exist, open a local copy as binary
    fp = fopen(filename, "wb+");
    if (!fp) {
        free(filename);
        return -1;
    }

    // Loop over each byte and write the received byte to the local file
    for (i = 0; i < file_length; ++i) {

        // Receive a single byte
        recv_len = recv_byte(sock, &current_byte);
        if (recv_len <= 0) {
            fclose(fp);
            free(filename);
            return -1;
        }

        // Write the byte locally
        fwrite(&current_byte, 1, 1, fp);

#if VERBOSE
        printf("\rReceiving %s | %d bytes | %d%%... ", filename, file_length,
                (int) (100 * floor((i + 1) / file_length)));
        fflush(stdout);
#endif

    }

#if VERBOSE
    printf("Done.\n");
#endif

    // Close the local file when finished
    fclose(fp);
    free(filename);
    return 0;
}
