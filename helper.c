
#include "helper.h"

int read_line(int sock, char *buffer, int size)
{
    int len = 0;
    char c;
    int ret;

    while (len < size)
    {
        ret = recv(sock, &c, 1, 0);
        if (ret <= 0) {
            buffer[len] = 0;
            return len;
        } else if (c == '\n') {
            buffer[len] = 0;
            return len;
        }

        if (c == '\0' && len == 0) {
            continue;
        }

        buffer[len++] = c;
    }

    return -1;
}

