#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

struct TestCase { const char *name; void (*fn)(void); const char *file; };
static struct TestCase _tests[4096];
static size_t _ntests;

int g_expect_failed;

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

/* Run one test in a forked child; parent reaps and reports via fd 3. */
static int run_one(const char *func, void (*fn)(void)) {
    int p[2];
    if (pipe(p) != 0) return -1;
    pid_t pid = fork();
    if (pid == 0) {
        close(p[0]);
        if (dup2(p[1], 3) < 0) _exit(2);
        close(p[1]);
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
    close(p[1]);
    char buf[256];
    ssize_t r;
    while ((r = read(p[0], buf, sizeof buf)) > 0) { /* parent parses later */ }
    close(p[0]);
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) return -1;
    if (WIFSIGNALED(status)) {
        fprintf(stderr, "[child %s] signal %d\n", func, WTERMSIG(status));
        send_result(func, "crash", V_CRASH, "signal");
        return -1;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        send_result(func, "ok", V_PASS, "");
        return 0;
    }
    send_result(func, "fail", V_FAIL, "non-zero exit");
    return 1;
}

int main(void) {
    size_t pass = 0, fail = 0;
    for (size_t i = 0; i < _ntests; i++) {
        int rc = run_one(_tests[i].name, _tests[i].fn);
        if (rc == 0) pass++; else fail++;
    }
    fprintf(stderr, "harness: %zu tests, %zu pass, %zu fail\n", _ntests, pass, fail);
    return fail ? 1 : 0;
}
