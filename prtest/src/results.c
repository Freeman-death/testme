#include "prtest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static Verdict parse_verdict(const char *s) {
    if (strcmp(s, "PASS") == 0) return V_PASS;
    if (strcmp(s, "FAIL") == 0) return V_FAIL;
    if (strcmp(s, "CRASH") == 0) return V_CRASH;
    if (strcmp(s, "TIMEOUT") == 0) return V_TIMEOUT;
    if (strcmp(s, "LEAK") == 0) return V_LEAK;
    if (strcmp(s, "MEMORY") == 0) return V_MEMORY;
    if (strcmp(s, "WARN") == 0) return V_WARN;
    return V_FAIL;
}

void results_parse(int fd, const Options *o, const ProjectInfo *info) {
    (void)info;
    char buf[4096];
    size_t buf_len = 0;

    while (1) {
        ssize_t r = read(fd, buf + buf_len, sizeof buf - buf_len - 1);
        if (r <= 0) break;
        buf_len += r;
        buf[buf_len] = '\0';

        char *start = buf;
        char *end;
        while ((end = strchr(start, '\n')) != NULL) {
            *end = '\0';
            if (strlen(start) > 0) {
                char *copy = strdup(start);
                char *parts[5];
                int n = 0;
                char *tok = strtok(copy, "\t");
                while (tok && n < 5) {
                    parts[n++] = tok;
                    tok = strtok(NULL, "\t");
                }
                if (n >= 4 && strcmp(parts[0], "RESULT") == 0) {
                    const char *func = parts[1];
                    const char *test = parts[2];
                    Verdict v = parse_verdict(parts[3]);
                    const char *detail = n >= 5 ? parts[4] : "";
                    report_result(o, func, test, v, detail);
                }
                free(copy);
            }
            start = end + 1;
        }
        if (start != buf) {
            size_t remaining = buf_len - (start - buf);
            memmove(buf, start, remaining);
            buf_len = remaining;
        }
    }

    if (buf_len > 0) {
        buf[buf_len] = '\0';
        char *copy = strdup(buf);
        char *parts[5];
        int n = 0;
        char *tok = strtok(copy, "\t");
        while (tok && n < 5) {
            parts[n++] = tok;
            tok = strtok(NULL, "\t");
        }
        if (n >= 4 && strcmp(parts[0], "RESULT") == 0) {
            const char *func = parts[1];
            const char *test = parts[2];
            Verdict v = parse_verdict(parts[3]);
            const char *detail = n >= 5 ? parts[4] : "";
            report_result(o, func, test, v, detail);
        }
        free(copy);
    }
}