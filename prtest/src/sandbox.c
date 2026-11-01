#include "prtest.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int sandbox_copy(const char *src, char *dst, size_t dstsz) {
    /* TODO subagent 1: recursive copy src -> /tmp/prtest-sb-<pid> */
    snprintf(dst, dstsz, "/tmp/prtest-sb-%d", (int)getpid());
    (void)src;
    return 0;
}

int build_sandbox(const char *sandbox, const Options *o) {
    /* TODO subagent 1: run 'make' in sandbox, capture output, verify libft.a */
    (void)sandbox; (void)o;
    return 0;
}
