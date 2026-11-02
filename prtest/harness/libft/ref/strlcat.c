#include "strlcat.h"
#include <string.h>

size_t ref_strlcat(char *dst, const char *src, size_t size)
{
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);
    if (size <= dst_len)
        return size + src_len;
    size_t copy_len = src_len < size - dst_len - 1 ? src_len : size - dst_len - 1;
    memcpy(dst + dst_len, src, copy_len);
    dst[dst_len + copy_len] = '\0';
    return dst_len + src_len;
}