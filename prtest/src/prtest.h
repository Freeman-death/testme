#ifndef PRTEST_H
#define PRTEST_H
#include <stddef.h>

/* ---- CLI options ---- */
typedef struct {
    const char *path;
    const char *project;
    const char *function;
    int verbose, strict, json, no_color, keep_sandbox, list_only;
    int only_functional, only_leaks, only_alloc_fail, only_build;
    int probe_ub, stress, valgrind, sanitize;
    unsigned int seed;
    double timeout_s;
} Options;

typedef enum { DETECT_OK, DETECT_WARN, DETECT_FAIL } DetectStatus;
typedef struct {
    const char *name;
    DetectStatus status;
    int has_makefile, has_header, n_ft_files;
    const char *reason;
} ProjectInfo;

typedef enum { V_PASS=0, V_FAIL, V_CRASH, V_TIMEOUT, V_LEAK, V_MEMORY, V_WARN } Verdict;

int  cli_parse(int argc, char **argv, Options *o);
int  detect_project(const char *path, ProjectInfo *info);
int  sandbox_copy(const char *src, char *dst, size_t dstsz);
int  run_cmd(const char *dir, char **argv, double timeout_s, int *out, char *err, size_t errsz);
int  build_sandbox(const char *sandbox, const Options *o);
void report_init(const Options *o, const ProjectInfo *info);
void report_finish(const Options *o);
void report_result(const Options *o, const char *func, const char *test, Verdict v, const char *detail);
int  run_harness(const char *sandbox, const Options *o, const ProjectInfo *info);

#endif
