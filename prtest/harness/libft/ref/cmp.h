#ifndef REF_CMP_H
#define REF_CMP_H
#include <stddef.h>

int ref_strncmp_sign(const char *s1, const char *s2, size_t n);
int ref_memcmp_sign(const void *s1, const void *s2, size_t n);

#endif