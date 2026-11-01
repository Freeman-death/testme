#include "protocol.h"
#include <stdio.h>
#include <string.h>

/* Failure flag read by the forked child in runner.c. */
int g_expect_failed;

void expect_true(const char *expr, int cond, const char *file, int line) {
    if (!cond) {
        g_expect_failed = 1;
        fprintf(stderr, "  FAIL %s:%d  %s\n", file, line, expr);
    }
}

void expect_eq_int(const char *expr, long got, long want, const char *file, int line) {
    if (got != want) {
        g_expect_failed = 1;
        fprintf(stderr, "  FAIL %s:%d  %s (got %ld, want %ld)\n", file, line, expr, got, want);
    }
}

void expect_ptr_in(const char *expr, const void *got, const void *lo, const void *hi,
                   const char *file, int line) {
    if (got < lo || got >= hi) {
        g_expect_failed = 1;
        fprintf(stderr, "  FAIL %s:%d  %s (pointer outside guard region)\n", file, line, expr);
    }
}

void expect_mem_eq(const char *expr, const void *got, const void *want, size_t n,
                   const char *file, int line) {
    if (memcmp(got, want, n) != 0) {
        g_expect_failed = 1;
        fprintf(stderr, "  FAIL %s:%d  %s (memory mismatch)\n", file, line, expr);
    }
}

void fail(const char *msg, const char *file, int line) {
    g_expect_failed = 1;
    fprintf(stderr, "  FAIL %s:%d  %s\n", file, line, msg);
}
