#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n)
{
    char *copy;
    size_t len;

    if (!s) {
        return NULL;  // Handle NULL input safely
    }

    // Determine the number of characters to copy (up to n or up to '\0')
    len = 0;
    while (len < n && s[len] != '\0') {
        len++;
    }

    // Prevent overflow: len + 1 must not overflow size_t
    if (len == SIZE_MAX) {
        return NULL; // extremely unlikely but technically correct
    }

    copy = (char *)malloc(len + 1);
    if (!copy) {
        return NULL;
    }

    memcpy(copy, s, len);
    copy[len] = '\0';
    return copy;
}
#endif


#ifndef HAVE_L64A
char *l64a(long value)
{
    static char buf[7];
    static const char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0;
    if (value == 0) {
        buf[i++] = '.';
    } else {        while (value && i < 6) {
            buf[i++] = tbl[value & 0x3F];
            value >>= 6;
        }
    }
    buf[i] = '\0';
    return buf;
}
#endif
