#include "cmp.h"

int ref_strncmp_sign(const char *s1, const char *s2, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        unsigned char c1 = (unsigned char)s1[i];
        unsigned char c2 = (unsigned char)s2[i];
        if (c1 != c2)
            return (c1 > c2) ? 1 : -1;
        if (c1 == '\0')
            break;
    }
    return 0;
}

int ref_memcmp_sign(const void *s1, const void *s2, size_t n)
{
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i])
            return (a[i] > b[i]) ? 1 : -1;
    }
    return 0;
}