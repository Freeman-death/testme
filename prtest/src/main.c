#include "prtest.h"
#include <stdio.h>

int cli_parse(int argc, char **argv, Options *o) {
    /* TODO subagent 1: real argv parsing (hand-rolled, no getopt_long on macOS). */
    (void)argc; (void)argv;
    o->path = ".";
    o->project = "libft";
    o->function = NULL;
    o->verbose = o->strict = o->json = o->no_color = 0;
    o->keep_sandbox = o->list_only = 0;
    o->only_functional = o->only_leaks = o->only_alloc_fail = o->only_build = 0;
    o->probe_ub = o->stress = o->valgrind = o->sanitize = 0;
    o->seed = 0x2f9c41aa;
    o->timeout_s = 2.0;
    return 0;
}

int run_harness(const char *sandbox, const Options *o, const ProjectInfo *info) {
    /* TODO subagent 1: compile harness against sandbox/libft.a, run it,
       parse fd 3 results, drive the report. */
    (void)sandbox; (void)o; (void)info;
    return 0;
}

int main(int argc, char **argv) {
    Options o; ProjectInfo info;
    if (cli_parse(argc, argv, &o)) return 2;
    if (detect_project(o.path, &info) != 0) {
        fprintf(stderr, "prtest: detection failed on '%s'\n", o.path);
        return 2;
    }
    report_init(&o, &info);
    char sb[1024];
    if (sandbox_copy(o.path, sb, sizeof sb) != 0) {
        fprintf(stderr, "prtest: sandbox copy failed\n");
        return 2;
    }
    if (build_sandbox(sb, &o) != 0) {
        fprintf(stderr, "prtest: build failed\n");
        report_finish(&o);
        return 2;
    }
    int rc = run_harness(sb, &o, &info);
    report_finish(&o);
    return rc;
}
