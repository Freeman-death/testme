#include "prtest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <time.h>

int run_cmd(const char *dir, char **argv, double timeout_s,
            int *out, char *err, size_t errsz) {
    int stdout_pipe[2], stderr_pipe[2];
    if (pipe(stdout_pipe) < 0 || pipe(stderr_pipe) < 0) {
        if (stdout_pipe[0] >= 0) { close(stdout_pipe[0]); close(stdout_pipe[1]); }
        if (stderr_pipe[0] >= 0) { close(stderr_pipe[0]); close(stderr_pipe[1]); }
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(stdout_pipe[0]); close(stdout_pipe[1]);
        close(stderr_pipe[0]); close(stderr_pipe[1]);
        return -1;
    }

    if (pid == 0) {
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        if (dir && chdir(dir) < 0) _exit(127);

        struct rlimit rl;
        rl.rlim_cur = rl.rlim_max = 256 * 1024 * 1024;
        setrlimit(RLIMIT_AS, &rl);
        rl.rlim_cur = rl.rlim_max = 0;
        setrlimit(RLIMIT_CORE, &rl);

        execvp(argv[0], argv);
        _exit(127);
    }

    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    struct pollfd pfds[2];
    pfds[0].fd = stdout_pipe[0];
    pfds[0].events = POLLIN;
    pfds[1].fd = stderr_pipe[0];
    pfds[1].events = POLLIN;

    char *stdout_buf = NULL;
    size_t stdout_cap = 0, stdout_len = 0;
    size_t stderr_len = 0;

    double start = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    start = ts.tv_sec + ts.tv_nsec * 1e-9;

    int status = 0;
    int timed_out = 0;
    int child_exited = 0;

    while (!child_exited) {
        double now;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        now = ts.tv_sec + ts.tv_nsec * 1e-9;
        double remaining = timeout_s - (now - start);
        if (remaining <= 0) {
            timed_out = 1;
            kill(pid, SIGKILL);
            break;
        }

        int poll_timeout = (int)(remaining * 1000);
        if (poll_timeout < 10) poll_timeout = 10;

        int pret = poll(pfds, 2, poll_timeout);
        if (pret < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (pret == 0) continue;

        for (int i = 0; i < 2; i++) {
            if (pfds[i].revents & POLLIN) {
                char buf[4096];
                ssize_t r = read(pfds[i].fd, buf, sizeof buf);
                if (r > 0) {
                    if (i == 0) {
                        if (stdout_len + r + 1 > stdout_cap) {
                            stdout_cap = stdout_cap ? stdout_cap * 2 : 4096;
                            while (stdout_len + r + 1 > stdout_cap) stdout_cap *= 2;
                            stdout_buf = realloc(stdout_buf, stdout_cap);
                        }
                        memcpy(stdout_buf + stdout_len, buf, r);
                        stdout_len += r;
                    } else {
                        if (stderr_len + r < errsz - 1) {
                            memcpy(err + stderr_len, buf, r);
                            stderr_len += r;
                        }
                    }
                } else if (r == 0) {
                    pfds[i].fd = -1;
                    pfds[i].events = 0;
                }
            }
        }

        int wstatus;
        pid_t w = waitpid(pid, &wstatus, WNOHANG);
        if (w == pid) {
            child_exited = 1;
            status = wstatus;
            break;
        }
    }

    close(stdout_pipe[0]);
    close(stderr_pipe[0]);

    if (!child_exited) {
        int wstatus;
        waitpid(pid, &wstatus, 0);
        status = wstatus;
    }

    if (err) {
        err[stderr_len] = '\0';
    }

    if (out && stdout_buf) {
        *out = stdout_len;
        free(stdout_buf);
    } else if (stdout_buf) {
        free(stdout_buf);
    }

    if (timed_out) return -2;
    if (WIFSIGNALED(status)) return -3;
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return -1;
}