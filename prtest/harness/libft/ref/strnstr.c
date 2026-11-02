#include "strnstr.h"
#include <string.h>

char *ref_strnstr(const char *hay, const char *need, size_t len)
{
    size_t need_len = strlen(need);
    if (need_len == 0)
        return (char *)hay;
    if (len == 0)
        return NULL;
    for (size_t i = 0; i + need_len <= len; i++) {
        if (hay[i] == '\0')
            break;
        if (memcmp(hay + i, need, need_len) == 0)
            return (char *)(hay + i);
    }
    return NULL;
}