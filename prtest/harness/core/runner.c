#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include <errno.h>

struct TestCase { const char *name; void (*fn)(void); const char *file; };
static struct TestCase _tests[4096];
static size_t _ntests;

int g_expect_failed;
static double g_timeout_s = 2.0;
static int g_verbose = 0;

void reg_test(const char *name, void (*fn)(void), const char *file) {
    if (_ntests < 4096) {
        _tests[_ntests].name = name;
        _tests[_ntests].fn = fn;
        _tests[_ntests].file = file;
        _ntests++;
    }
}

void send_result(const char *func, const char *test, Verdict v, const char *detail) {
    char buf[2048];
    const char *vn = v == V_PASS ? "PASS" : v == V_FAIL ? "FAIL" :
        v == V_CRASH ? "CRASH" : v == V_TIMEOUT ? "TIMEOUT" :
        v == V_LEAK ? "LEAK" : v == V_WARN ? "WARN" : "MEM";
    int n = snprintf(buf, sizeof buf, "RESULT\t%s\t%s\t%s\t%s\n",
                     func, test, vn, detail ? detail : "");
    if (n > 0) { ssize_t w = write(3, buf, (size_t)n); (void)w; }
}

static int run_one(const char *func, void (*fn)(void)) {
    pid_t pid = fork();
    if (pid == 0) {
        awrap_install();
        awrap_reset();
        g_expect_failed = 0;
        fn();
        if (g_expect_failed)
            send_result(func, "expect", V_FAIL, "expectation failed");
        else if (awrap_live_blocks() != 0)
            send_result(func, "leak", V_LEAK, "live blocks after test");
        else
            send_result(func, "ok", V_PASS, "");
        _exit(g_expect_failed ? 1 : 0);
    }
    
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) * 1e-9;
        if (elapsed >= g_timeout_s) {
            kill(pid, SIGKILL);
            send_result(func, "timeout", V_TIMEOUT, "test timed out");
            return -1;
        }
        int remaining_ms = (int)((g_timeout_s - elapsed) * 1000);
        if (remaining_ms < 1) remaining_ms = 1;
        
        int wstatus;
        pid_t w = waitpid(pid, &wstatus, WNOHANG);
        if (w == pid) {
            if (WIFSIGNALED(wstatus)) {
                char detail[128];
                snprintf(detail, sizeof detail, "signal %d", WTERMSIG(wstatus));
                send_result(func, "crash", V_CRASH, detail);
                return -1;
            }
            if (WIFEXITED(wstatus)) {
                return 0;
            }
            return -1;
        }
        usleep(1000);
    }
}

static int parse_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            g_verbose = 1;
        } else if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc) {
            g_timeout_s = strtod(argv[++i], NULL);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            rng_seed(strtoull(argv[++i], NULL, 0));
        } else if (strcmp(argv[i], "--strict") == 0) {
        } else if (strcmp(argv[i], "--probe-ub") == 0) {
        } else if (strcmp(argv[i], "--stress") == 0) {
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    parse_args(argc, argv);
    size_t pass = 0, fail = 0;
    for (size_t i = 0; i < _ntests; i++) {
        int rc = run_one(_tests[i].name, _tests[i].fn);
        if (rc == 0) pass++; else fail++;
    }
    if (g_verbose) {
        fprintf(stderr, "harness: %zu tests, %zu pass, %zu fail\n", _ntests, pass, fail);
    }
    return fail ? 1 : 0;
}