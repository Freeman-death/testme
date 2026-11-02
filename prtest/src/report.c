#include "prtest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    const char *func;
    const char *test;
    Verdict v;
    char detail[512];
} ResultEntry;

static ResultEntry *g_results = NULL;
static size_t g_nresults = 0, g_cap = 0;
static int g_color = 0;
static size_t g_pass = 0, g_fail = 0, g_crash = 0, g_timeout = 0, g_leak = 0, g_warn = 0;
static const Options *g_opts = NULL;

static const char *verdict_str(Verdict v) {
    switch (v) {
        case V_PASS: return "PASS";
        case V_FAIL: return "FAIL";
        case V_CRASH: return "CRASH";
        case V_TIMEOUT: return "TIMEOUT";
        case V_LEAK: return "LEAK";
        case V_MEMORY: return "MEMORY";
        case V_WARN: return "WARN";
    }
    return "UNKNOWN";
}

static const char *verdict_mark(Verdict v) {
    switch (v) {
        case V_PASS: return g_color ? "\xe2\x9c\x93" : "[OK]";
        case V_FAIL: return g_color ? "\xe2\x9c\x97" : "[FAIL]";
        case V_CRASH: return g_color ? "\xf0\x9f\x92\xa5" : "[CRASH]";
        case V_TIMEOUT: return g_color ? "\xe2\x8f\xb0" : "[TIMEOUT]";
        case V_LEAK: return g_color ? "\xf0\x9f\x92\xa7" : "[LEAK]";
        case V_MEMORY: return g_color ? "\xe2\x9a\xa0" : "[MEM]";
        case V_WARN: return g_color ? "\xe2\x9a\xa0" : "[WARN]";
    }
    return "?";
}

static const char *verdict_color(Verdict v) {
    if (!g_color) return "";
    switch (v) {
        case V_PASS: return "\x1b[32m";
        case V_FAIL: return "\x1b[31m";
        case V_CRASH: return "\x1b[35m";
        case V_TIMEOUT: return "\x1b[33m";
        case V_LEAK: return "\x1b[36m";
        case V_MEMORY: return "\x1b[31m";
        case V_WARN: return "\x1b[33m";
    }
    return "";
}

static const char *reset_color(void) { return g_color ? "\x1b[0m" : ""; }

void report_init(const Options *o, const ProjectInfo *info) {
    g_color = !o->no_color && isatty(1) && !getenv("NO_COLOR");
    g_opts = o;
    if (g_results) { free(g_results); g_results = NULL; }
    g_nresults = g_cap = 0;
    g_pass = g_fail = g_crash = g_timeout = g_leak = g_warn = 0;
    
    const char *status_str = info->status == DETECT_OK ? "OK" : 
                             info->status == DETECT_WARN ? "WARN" : "FAIL";
    printf("prtest: testing %s at %s [%s]\n", info->name, o->path, status_str);
    if (info->reason) printf("  detection: %s\n", info->reason);
}

void report_result(const Options *o, const char *func, const char *test, Verdict v, const char *detail) {
    (void)o;
    if (g_nresults >= g_cap) {
        g_cap = g_cap ? g_cap * 2 : 64;
        g_results = realloc(g_results, g_cap * sizeof(ResultEntry));
    }
    ResultEntry *e = &g_results[g_nresults++];
    e->func = func;
    e->test = test;
    e->v = v;
    snprintf(e->detail, sizeof e->detail, "%s", detail ? detail : "");

    switch (v) {
        case V_PASS: g_pass++; break;
        case V_FAIL: g_fail++; break;
        case V_CRASH: g_crash++; break;
        case V_TIMEOUT: g_timeout++; break;
        case V_LEAK: g_leak++; break;
        case V_WARN: g_warn++; break;
        default: break;
    }

    if (!g_opts->json) {
        const char *color = verdict_color(v);
        const char *reset = reset_color();
        if (g_opts->failed_only && v == V_PASS) return;
        printf("  %s%s%s %s/%s\n", color, verdict_mark(v), reset, func, test);
    }
}

void report_finish(const Options *o) {
    if (o->json) {
        printf("{\n  \"results\": [\n");
        for (size_t i = 0; i < g_nresults; i++) {
            ResultEntry *e = &g_results[i];
            printf("    {\"func\": \"%s\", \"test\": \"%s\", \"verdict\": \"%s\", \"detail\": \"%s\"}%s\n",
                   e->func, e->test, verdict_str(e->v), e->detail, i + 1 < g_nresults ? "," : "");
        }
        printf("  ],\n  \"summary\": {\n");
        printf("    \"pass\": %zu,\n    \"fail\": %zu,\n    \"crash\": %zu,\n", g_pass, g_fail, g_crash);
        printf("    \"timeout\": %zu,\n    \"leak\": %zu,\n    \"warn\": %zu\n", g_timeout, g_leak, g_warn);
        printf("  }\n}\n");
    } else {
        if (g_fail || g_crash || g_timeout || g_leak || g_warn) {
            printf("\nFailures:\n");
            for (size_t i = 0; i < g_nresults; i++) {
                ResultEntry *e = &g_results[i];
                if (e->v != V_PASS) {
                    const char *color = verdict_color(e->v);
                    const char *reset = reset_color();
                    printf("  %s%s%s %s/%s\n", color, verdict_mark(e->v), reset, e->func, e->test);
                    if (e->detail[0]) {
                        printf("    %s\n", e->detail);
                    }
                }
            }
        }
        printf("\nSummary: %zu pass, %zu fail, %zu crash, %zu timeout, %zu leak, %zu warn\n",
               g_pass, g_fail, g_crash, g_timeout, g_leak, g_warn);
    }

    if (g_results) { free(g_results); g_results = NULL; }
    g_nresults = g_cap = 0;
}