#include "prtest.h"
#include <stddef.h>

int run_cmd(const char *dir, char **argv, double timeout_s,
            int *out, char *err, size_t errsz) {
    /* TODO subagent 1: fork/execvp with pipe capture, timeout via
       waitpid(WNOHANG) poll + kill, RLIMIT_AS, temp cwd. */
    (void)dir; (void)argv; (void)timeout_s; (void)err; (void)errsz;
    if (out) *out = 0;
    return 0;
}
