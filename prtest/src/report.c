#include "prtest.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int g_color;

void report_init(const Options *o, const ProjectInfo *info) {
    g_color = !o->no_color && isatty(1) && !getenv("NO_COLOR");
    (void)info;
}

void report_result(const Options *o, const char *func, const char *test,
                   Verdict v, const char *detail) {
    const char *mark = v == V_PASS ? "\xe2\x9c\x93" :
                       v == V_FAIL ? "\xe2\x9c\x97" : "!";
    (void)o; (void)test; (void)detail;
    if (g_color) printf("  %s %s\n", mark, func);
    else         printf("  %s %s\n", mark, func);
}

void report_finish(const Options *o) {
    (void)o;
    printf("prtest: report complete\n");
}
