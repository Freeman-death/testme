#include "prtest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>

/* Forward declarations from spec.c (owned by subagent 2, not in prtest.h) */
typedef enum {
    CAT_CTYPE   = 0,
    CAT_STRING  = 1,
    CAT_MEMORY  = 2,
    CAT_CONVERT = 3,
    CAT_ALLOC   = 4,
    CAT_OUTPUT  = 5,
    CAT_LIST    = 6,
    CAT_COUNT
} SpecCategory;

typedef enum {
    TIER_A = 0,
    TIER_B = 1,
    TIER_C = 2
} SpecTier;

#define MAX_EXTERNALS 4

typedef struct {
    const char *name;
    const char *prototype;
    SpecCategory category;
    SpecTier tier;
    const char *externals[MAX_EXTERNALS];
} SpecEntry;

extern const SpecEntry *spec_find(const char *name);
extern size_t spec_count(void);
extern const SpecEntry *spec_all(void);
extern const char *spec_category_name(SpecCategory cat);
extern const char *spec_tier_name(SpecTier tier);

/* ---- ReportSink callback for static check results ---- */
typedef enum {
    STAT_PASS = 0,
    STAT_FAIL,
    STAT_WARN
} StaticVerdict;

typedef struct {
    StaticVerdict verdict;
    const char *check_name;
    const char *message;
} StaticResult;

typedef void (*ReportSink)(const StaticResult *result, void *user_data);

/* ---- Internal helpers ---- */

/* Run a command via popen and capture output */
static char *run_cmd_capture(const char *cmd, size_t *out_len)
{
    FILE *fp = popen(cmd, "r");
    if (!fp)
        return NULL;

    size_t cap = 4096;
    char *buf = malloc(cap);
    if (!buf) {
        pclose(fp);
        return NULL;
    }

    size_t len = 0;
    char chunk[1024];
    while (fgets(chunk, sizeof(chunk), fp)) {
        size_t clen = strlen(chunk);
        if (len + clen + 1 >= cap) {
            cap *= 2;
            char *nb = realloc(buf, cap);
            if (!nb) {
                free(buf);
                pclose(fp);
                return NULL;
            }
            buf = nb;
        }
        memcpy(buf + len, chunk, clen);
        len += clen;
    }
    pclose(fp);
    buf[len] = '\0';
    if (out_len)
        *out_len = len;
    return buf;
}

/* Check if a symbol is a compiler runtime symbol (whitelisted) */
static int is_compiler_runtime_symbol(const char *sym)
{
    if (!sym)
        return 0;
    /* Skip leading underscores for macOS */
    while (sym[0] == '_')
        sym++;
    if (strncmp(sym, "stack_chk_fail", 14) == 0)
        return 1;
    if (strncmp(sym, "memcpy$", 7) == 0)
        return 1;
    if (strncmp(sym, "memset$", 7) == 0)
        return 1;
    if (strncmp(sym, "memmove$", 8) == 0)
        return 1;
    if (strncmp(sym, "memcmp$", 7) == 0)
        return 1;
    if (strncmp(sym, "strlen$", 7) == 0)
        return 1;
    if (strncmp(sym, "malloc$", 7) == 0)
        return 1;
    if (strncmp(sym, "free$", 5) == 0)
        return 1;
    if (strncmp(sym, "write$", 6) == 0)
        return 1;
    /* Fortified/checked variants inserted by compiler at -O2 -D_FORTIFY_SOURCE */
    if (strncmp(sym, "memset_chk", 10) == 0)
        return 1;
    if (strncmp(sym, "memcpy_chk", 10) == 0)
        return 1;
    if (strncmp(sym, "memmove_chk", 11) == 0)
        return 1;
    if (strncmp(sym, "stpcpy_chk", 10) == 0)
        return 1;
    if (strncmp(sym, "strcpy_chk", 10) == 0)
        return 1;
    if (strncmp(sym, "strncpy_chk", 11) == 0)
        return 1;
    if (strncmp(sym, "strcat_chk", 10) == 0)
        return 1;
    if (strncmp(sym, "strncat_chk", 11) == 0)
        return 1;
    if (strncmp(sym, "sprintf_chk", 11) == 0)
        return 1;
    if (strncmp(sym, "snprintf_chk", 12) == 0)
        return 1;
    return 0;
}

/* Check if a symbol is in the allowed externals list for a function */
static int is_allowed_external(const SpecEntry *entry, const char *sym)
{
    if (!entry || !sym)
        return 0;
    for (int i = 0; i < MAX_EXTERNALS && entry->externals[i]; i++) {
        if (strcmp(entry->externals[i], sym) == 0)
            return 1;
    }
    return 0;
}

/* Normalize symbol: strip leading underscore on macOS */
static const char *norm_sym(const char *sym)
{
    if (sym && sym[0] == '_')
        return sym + 1;
    return sym;
}

/* ---- Check 1: nm -g --defined-only libft.a - implemented symbols ---- */
static void check_implemented_symbols(const char *sandbox, ReportSink sink, void *ud)
{
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "nm -g --defined-only %s/libft.a 2>/dev/null", sandbox);
    char *out = run_cmd_capture(cmd, NULL);
    StaticResult r = { .check_name = "implemented_symbols" };

    if (!out) {
        r.verdict = STAT_FAIL;
        r.message = "nm failed or libft.a not found";
        sink(&r, ud);
        return;
    }

    /* Parse defined symbols */
    int found[43] = {0};
    int extra_non_ft = 0;
    char line[256];
    char *p = out;
    while (*p) {
        char *eol = strchr(p, '\n');
        size_t linelen = eol ? (size_t)(eol - p) : strlen(p);
        if (linelen < sizeof(line) - 1) {
            memcpy(line, p, linelen);
            line[linelen] = '\0';
            /* Look for " T _ft_" */
            char *t_pos = strstr(line, " T _ft_");
            if (t_pos) {
                const char *sym = t_pos + 4; /* skip " T " */
                sym = norm_sym(sym);
                const SpecEntry *e = spec_find(sym);
                if (e) {
                    for (size_t i = 0; i < spec_count(); i++) {
                        if (strcmp(spec_all()[i].name, sym) == 0) {
                            found[i] = 1;
                            break;
                        }
                    }
                } else {
                    extra_non_ft++;
                }
            }
        }
        if (eol)
            p = eol + 1;
        else
            break;
    }
    free(out);

    /* Check for missing functions */
    int missing = 0;
    for (size_t i = 0; i < spec_count(); i++) {
        if (!found[i]) {
            missing++;
            r.verdict = STAT_FAIL;
            char msg[256];
            snprintf(msg, sizeof(msg), "Missing required function: %s", spec_all()[i].name);
            r.message = msg;
            sink(&r, ud);
        }
    }

    if (extra_non_ft > 0) {
        r.verdict = STAT_WARN;
        char msg[256];
        snprintf(msg, sizeof(msg), "Extra non-ft_ symbols in libft.a: %d", extra_non_ft);
        r.message = msg;
        sink(&r, ud);
    }

    if (missing == 0 && extra_non_ft == 0) {
        r.verdict = STAT_PASS;
        r.message = "All 43 required functions present, no extra symbols";
        sink(&r, ud);
    }
}

/* ---- Check 2: nm -u per object - undefined externals ---- */
static void check_undefined_externals(const char *sandbox, ReportSink sink, void *ud)
{
    /* First, list object files in the sandbox */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "ls %s/*.o 2>/dev/null", sandbox);
    char *out = run_cmd_capture(cmd, NULL);
    if (!out) {
        StaticResult r = { .check_name = "undefined_externals",
            .verdict = STAT_WARN, .message = "No object files found in sandbox" };
        sink(&r, ud);
        return;
    }

    StaticResult r = { .check_name = "undefined_externals" };
    int any_issue = 0;

    char *saveptr;
        char *obj = strtok_r(out, "\n", &saveptr);
        while (obj) {
            if (obj[0] != '\0') {
                /* Make a copy since we'll modify it */
                char obj_copy[1024];
                strncpy(obj_copy, obj, sizeof(obj_copy) - 1);
                obj_copy[sizeof(obj_copy) - 1] = '\0';
                char *base = strrchr(obj_copy, '/');
                base = base ? base + 1 : obj_copy;
                char *dot = strrchr(base, '.');
                if (dot)
                    *dot = '\0';
                const SpecEntry *entry = spec_find(base);
                if (!entry) {
                    obj = strtok_r(NULL, "\n", &saveptr);
                    continue;
                }
                snprintf(cmd, sizeof(cmd), "nm -u %s 2>/dev/null", obj);
                char *uout = run_cmd_capture(cmd, NULL);
                if (uout) {
                    char line[256];
                    char *p = uout;
                    while (*p) {
                        char *eol = strchr(p, '\n');
                        size_t linelen = eol ? (size_t)(eol - p) : strlen(p);
                        if (linelen < sizeof(line) - 1) {
                            memcpy(line, p, linelen);
                            line[linelen] = '\0';
                            /* Line format: "symbol_name" (nm -u outputs just symbol names, one per line) */
                            char *sym = line;
                            /* Trim leading whitespace */
                            while (*sym && isspace(*sym))
                                sym++;
                            /* Trim trailing whitespace */
                            char *end = sym + strlen(sym) - 1;
                            while (end > sym && isspace(*end))
                                *end-- = '\0';
                            if (*sym == '\0') {
                                if (eol) p = eol + 1; else break;
                                continue;
                            }
                            const char *norm = norm_sym(sym);
                            if (!is_allowed_external(entry, norm)) {
                                /* Skip internal ft_* calls - they're implementation details */
                                if (strncmp(norm, "ft_", 3) != 0) {
                                    any_issue = 1;
                                    r.verdict = STAT_WARN;
                                    char msg[512];
                                    const char *hint = "";
                                    if (is_compiler_runtime_symbol(norm))
                                        hint = " (hint: compiler may have turned a loop into a libc call at -O2)";
                                    snprintf(msg, sizeof(msg),
                                        "%s: uses undefined external '%s' (not in allowed list%s)",
                                        entry->name, norm, hint);
                                    r.message = msg;
                                    sink(&r, ud);
                                }
                            }
                        }
                        if (eol) p = eol + 1; else break;
                    }
                    free(uout);
                }
            }
            obj = strtok_r(NULL, "\n", &saveptr);
        }
    free(out);

    if (!any_issue) {
        r.verdict = STAT_PASS;
        r.message = "No disallowed undefined externals found";
        sink(&r, ud);
    }
}

/* ---- Check 3: main symbol inside the lib ---- */
static void check_main_symbol(const char *sandbox, ReportSink sink, void *ud)
{
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "nm -g %s/libft.a 2>/dev/null | grep ' T _main$'", sandbox);
    char *out = run_cmd_capture(cmd, NULL);
    StaticResult r = { .check_name = "main_symbol" };

    if (out && strlen(out) > 0) {
        r.verdict = STAT_FAIL;
        r.message = "main symbol found in libft.a (leftover test file)";
        sink(&r, ud);
        free(out);
        return;
    }
    if (out)
        free(out);

    r.verdict = STAT_PASS;
    r.message = "No main symbol in libft.a";
    sink(&r, ud);
}

/* ---- Check 4: Non-static helper symbols ---- */
static void check_nonstatic_helpers(const char *sandbox, ReportSink sink, void *ud)
{
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "nm -g %s/libft.a 2>/dev/null | grep ' T ' | grep -v ' T _ft_'", sandbox);
    char *out = run_cmd_capture(cmd, NULL);
    StaticResult r = { .check_name = "nonstatic_helpers" };

    if (out && strlen(out) > 0) {
        r.verdict = STAT_WARN;
        r.message = "Non-static helper symbols found (subject requires helpers to be static)";
        sink(&r, ud);
        free(out);
        return;
    }
    if (out)
        free(out);

    r.verdict = STAT_PASS;
    r.message = "No non-static helper symbols";
    sink(&r, ud);
}

/* ---- Check 5: Global data symbols ---- */
static void check_global_data(const char *sandbox, ReportSink sink, void *ud)
{
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "nm -g %s/libft.a 2>/dev/null | grep ' [DB] '", sandbox);
    char *out = run_cmd_capture(cmd, NULL);
    StaticResult r = { .check_name = "global_data" };

    if (out && strlen(out) > 0) {
        r.verdict = STAT_FAIL;
        r.message = "Global data symbols found (globals forbidden by subject)";
        sink(&r, ud);
        free(out);
        return;
    }
    if (out)
        free(out);

    r.verdict = STAT_PASS;
    r.message = "No global data symbols";
    sink(&r, ud);
}

/* ---- Check 6: 'restrict' in libft.h ---- */
static void check_restrict_in_header(const char *sandbox, ReportSink sink, void *ud)
{
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "grep -n restrict %s/libft.h 2>/dev/null", sandbox);
    char *out = run_cmd_capture(cmd, NULL);
    StaticResult r = { .check_name = "restrict_in_header" };

    if (out && strlen(out) > 0) {
        r.verdict = STAT_WARN;
        r.message = "'restrict' keyword found in libft.h (not in subject v19.3)";
        sink(&r, ud);
        free(out);
        return;
    }
    if (out)
        free(out);

    r.verdict = STAT_PASS;
    r.message = "No 'restrict' in libft.h";
    sink(&r, ud);
}

/* ---- Check 7: libft.h self-contained (include guards, double include) ---- */
static void check_header_selfcontained(const char *sandbox, ReportSink sink, void *ud)
{
    char test_file[1024];
    snprintf(test_file, sizeof(test_file), "%s/test_header.c", sandbox);

    FILE *fp = fopen(test_file, "w");
    if (!fp) {
        StaticResult r = { .check_name = "header_selfcontained",
            .verdict = STAT_WARN, .message = "Cannot write test file" };
        sink(&r, ud);
        return;
    }
    fprintf(fp, "#include \"libft.h\"\n#include \"libft.h\"\nint main(void) { return 0; }\n");
    fclose(fp);

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "cd %s && cc -std=c11 -Wall -Wextra -Werror -I. -c test_header.c -o test_header.o 2>&1", sandbox);
    char *out = run_cmd_capture(cmd, NULL);
    StaticResult r = { .check_name = "header_selfcontained" };

    unlink(test_file);
    char *obj_file = malloc(strlen(test_file) + 3);
    if (obj_file) {
        strcpy(obj_file, test_file);
        strcat(obj_file, ".o");
        unlink(obj_file);
        free(obj_file);
    }

    if (out && strlen(out) > 0) {
        r.verdict = STAT_FAIL;
        r.message = "libft.h not self-contained (double include or missing guards fails)";
        sink(&r, ud);
        free(out);
        return;
    }
    if (out)
        free(out);

    /* Also check for include guards */
    snprintf(cmd, sizeof(cmd), "grep -E '^#ifndef|^# *define|^#endif' %s/libft.h 2>/dev/null | head -3", sandbox);
    out = run_cmd_capture(cmd, NULL);
    int has_guard = out && strstr(out, "#ifndef") && (strstr(out, "#define") || strstr(out, "# define")) && strstr(out, "#endif");
    if (out)
        free(out);

    if (!has_guard) {
        r.verdict = STAT_WARN;
        r.message = "libft.h missing standard include guards";
        sink(&r, ud);
        return;
    }

    r.verdict = STAT_PASS;
    r.message = "libft.h is self-contained with include guards";
    sink(&r, ud);
}

/* ---- Check 8: t_list layout matches subject ---- */
static void check_tlist_layout(const char *sandbox, ReportSink sink, void *ud)
{
    char test_file[1024];
    snprintf(test_file, sizeof(test_file), "%s/test_tlist.c", sandbox);

    FILE *fp = fopen(test_file, "w");
    if (!fp) {
        StaticResult r = { .check_name = "tlist_layout",
            .verdict = STAT_WARN, .message = "Cannot write test file" };
        sink(&r, ud);
        return;
    }
    fprintf(fp,
        "#include \"libft.h\"\n"
        "#include <stddef.h>\n"
        "int main(void) {\n"
        "    t_list *n = NULL;\n"
        "    (void)n;\n"
        "    (void)offsetof(t_list, content);\n"
        "    (void)offsetof(t_list, next);\n"
        "    return 0;\n"
        "}\n");
    fclose(fp);

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "cd %s && cc -std=c11 -Wall -Wextra -Werror -I. -c test_tlist.c -o test_tlist.o 2>&1", sandbox);
    char *out = run_cmd_capture(cmd, NULL);
    StaticResult r = { .check_name = "tlist_layout" };

    unlink(test_file);
    char *obj_file = malloc(strlen(test_file) + 3);
    if (obj_file) {
        strcpy(obj_file, test_file);
        strcat(obj_file, ".o");
        unlink(obj_file);
        free(obj_file);
    }

    if (out && strlen(out) > 0) {
        r.verdict = STAT_FAIL;
        r.message = "t_list layout mismatch (expected: void* content; t_list* next)";
        sink(&r, ud);
        free(out);
        return;
    }
    if (out)
        free(out);

    r.verdict = STAT_PASS;
    r.message = "t_list layout matches subject (content ptr + next ptr)";
    sink(&r, ud);
}

/* ---- Check 9: Makefile behavior checks ---- */
static void check_makefile_behavior(const char *sandbox, ReportSink sink, void *ud)
{
    StaticResult r = { .check_name = "makefile_behavior" };

    char cmd[1024];
    char *out;

    /* 9a: 'make' works and produces libft.a at repo root - verify Makefile has required structure */
    snprintf(cmd, sizeof(cmd), "grep -E '^all:' %s/Makefile 2>/dev/null", sandbox);
    out = run_cmd_capture(cmd, NULL);
    if (!out || strlen(out) == 0) {
        r.verdict = STAT_FAIL;
        r.message = "Makefile missing 'all' rule";
        sink(&r, ud);
        if (out) free(out);
        return;
    }
    free(out);

    /* Check libft.a exists at root (from previous build) */
    snprintf(cmd, sizeof(cmd), "test -f %s/libft.a", sandbox);
    if (system(cmd) != 0) {
        r.verdict = STAT_FAIL;
        r.message = "libft.a not found at repo root";
        sink(&r, ud);
        return;
    }

    /* 9b: second 'make' doesn't relink/recompile - check Makefile has proper dependencies */
    /* We can't easily test this without running make, so just check for .o dependency pattern */
    snprintf(cmd, sizeof(cmd), "grep -E '\\.o:' %s/Makefile 2>/dev/null", sandbox);
    out = run_cmd_capture(cmd, NULL);
    int has_deps = out && strlen(out) > 0;
    free(out);
    if (!has_deps) {
        r.verdict = STAT_WARN;
        r.message = "Makefile may not have proper .o dependencies for incremental builds";
        sink(&r, ud);
        /* continue */
    }

    /* 9c: flags include -Wall -Wextra -Werror and use cc */
    snprintf(cmd, sizeof(cmd), "grep -E 'CFLAGS.*=.*-Wall.*-Wextra.*-Werror' %s/Makefile 2>/dev/null", sandbox);
    out = run_cmd_capture(cmd, NULL);
    int has_flags = out && strlen(out) > 0;
    free(out);

    snprintf(cmd, sizeof(cmd), "grep -E '^CC.*=.*cc' %s/Makefile 2>/dev/null", sandbox);
    out = run_cmd_capture(cmd, NULL);
    int uses_cc = out && strlen(out) > 0;
    free(out);

    if (!has_flags || !uses_cc) {
        r.verdict = STAT_WARN;
        r.message = "Makefile missing -Wall -Wextra -Werror or doesn't use cc";
        sink(&r, ud);
        /* continue */
    }

    /* 9d: ar is used and libtool is NOT */
    snprintf(cmd, sizeof(cmd), "grep -E '^\\tar ' %s/Makefile 2>/dev/null", sandbox);
    out = run_cmd_capture(cmd, NULL);
    int uses_ar = out && strlen(out) > 0;
    free(out);

    snprintf(cmd, sizeof(cmd), "grep -i libtool %s/Makefile 2>/dev/null", sandbox);
    out = run_cmd_capture(cmd, NULL);
    int uses_libtool = out && strlen(out) > 0;
    free(out);

    if (!uses_ar || uses_libtool) {
        r.verdict = STAT_WARN;
        r.message = "Makefile doesn't use 'ar' or uses 'libtool'";
        sink(&r, ud);
        /* continue */
    }

    /* 9e: rules $(NAME)/all/clean/fclean/re work - just verify they exist in Makefile */
    const char *rules[] = { "all", "clean", "fclean", "re", NULL };
    for (int i = 0; rules[i]; i++) {
        snprintf(cmd, sizeof(cmd), "grep -E '^%s:' %s/Makefile 2>/dev/null", rules[i], sandbox);
        out = run_cmd_capture(cmd, NULL);
        if (!out || strlen(out) == 0) {
            r.verdict = STAT_WARN;
            char msg[256];
            snprintf(msg, sizeof(msg), "Makefile missing rule: %s", rules[i]);
            r.message = msg;
            sink(&r, ud);
        }
        if (out) free(out);
    }

    r.verdict = STAT_PASS;
    r.message = "Makefile behavior checks passed";
    sink(&r, ud);
}

/* ---- Aggregate: run all static checks ---- */
int static_run_all(const char *sandbox, ReportSink sink, void *user_data)
{
    if (!sandbox || !sink)
        return -1;

    check_implemented_symbols(sandbox, sink, user_data);
    check_undefined_externals(sandbox, sink, user_data);
    check_main_symbol(sandbox, sink, user_data);
    check_nonstatic_helpers(sandbox, sink, user_data);
    check_global_data(sandbox, sink, user_data);
    check_restrict_in_header(sandbox, sink, user_data);
    check_header_selfcontained(sandbox, sink, user_data);
    check_tlist_layout(sandbox, sink, user_data);
    check_makefile_behavior(sandbox, sink, user_data);

    return 0;
}