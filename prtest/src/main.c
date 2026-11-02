#include "prtest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [project] [path] [options]\n"
        "  project    project name (default: libft)\n"
        "  path       path to project (default: .)\n"
        "Options:\n"
        "  -f, --function NAME    run only this function\n"
        "  -c, --category NAME    run only this category\n"
        "  --only KIND            only functional|leaks|alloc_fail|build\n"
        "  -v, --verbose          verbose output\n"
        "  --failed-only          show only failures\n"
        "  --strict               treat warnings as failures\n"
        "  --probe-ub             run UB tests (tier C)\n"
        "  --stress               stress mode (more iterations)\n"
        "  --valgrind             run under valgrind\n"
        "  --sanitize             enable ASan/UBSan\n"
        "  --seed N               RNG seed (default: 0x2f9c41aa)\n"
        "  --timeout S            per-test timeout seconds (default: 2.0)\n"
        "  --json                 JSON output\n"
        "  --no-color             disable ANSI colors\n"
        "  --list                 list tests and exit\n"
        "  --keep-sandbox         don't delete sandbox on exit\n"
        "  -h, --help             show this help\n", prog);
}

int cli_parse(int argc, char **argv, Options *o) {
    o->path = ".";
    o->project = "libft";
    o->function = NULL;
    o->verbose = 0;
    o->strict = 0;
    o->json = 0;
    o->no_color = 0;
    o->keep_sandbox = 0;
    o->list_only = 0;
    o->failed_only = 0;
    o->only_functional = 0;
    o->only_leaks = 0;
    o->only_alloc_fail = 0;
    o->only_build = 0;
    o->probe_ub = 0;
    o->stress = 0;
    o->valgrind = 0;
    o->sanitize = 0;
    o->seed = 0x2f9c41aa;
    o->timeout_s = 2.0;

    int i = 1;
    int positional = 0;
    while (i < argc) {
        char *arg = argv[i];
        if (arg[0] == '-') {
            if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
                usage(argv[0]);
                return 2;
            } else if (strcmp(arg, "-f") == 0 || strcmp(arg, "--function") == 0) {
                if (++i >= argc) { fprintf(stderr, "Missing argument for %s\n", arg); return 2; }
                o->function = argv[i];
            } else if (strcmp(arg, "-c") == 0 || strcmp(arg, "--category") == 0) {
                if (++i >= argc) { fprintf(stderr, "Missing argument for %s\n", arg); return 2; }
                /* category filter not yet implemented in harness */
                (void)argv[i];
            } else if (strcmp(arg, "--only") == 0) {
                if (++i >= argc) { fprintf(stderr, "Missing argument for %s\n", arg); return 2; }
                if (strcmp(argv[i], "functional") == 0) o->only_functional = 1;
                else if (strcmp(argv[i], "leaks") == 0) o->only_leaks = 1;
                else if (strcmp(argv[i], "alloc_fail") == 0) o->only_alloc_fail = 1;
                else if (strcmp(argv[i], "build") == 0) o->only_build = 1;
                else { fprintf(stderr, "Unknown --only kind: %s\n", argv[i]); return 2; }
            } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
                o->verbose = 1;
            } else if (strcmp(arg, "--failed-only") == 0) {
                o->failed_only = 1;
            } else if (strcmp(arg, "--strict") == 0) {
                o->strict = 1;
            } else if (strcmp(arg, "--probe-ub") == 0) {
                o->probe_ub = 1;
            } else if (strcmp(arg, "--stress") == 0) {
                o->stress = 1;
            } else if (strcmp(arg, "--valgrind") == 0) {
                o->valgrind = 1;
            } else if (strcmp(arg, "--sanitize") == 0) {
                o->sanitize = 1;
            } else if (strcmp(arg, "--seed") == 0) {
                if (++i >= argc) { fprintf(stderr, "Missing argument for %s\n", arg); return 2; }
                o->seed = (unsigned int)strtoul(argv[i], NULL, 0);
            } else if (strcmp(arg, "--timeout") == 0) {
                if (++i >= argc) { fprintf(stderr, "Missing argument for %s\n", arg); return 2; }
                o->timeout_s = strtod(argv[i], NULL);
            } else if (strcmp(arg, "--json") == 0) {
                o->json = 1;
            } else if (strcmp(arg, "--no-color") == 0) {
                o->no_color = 1;
            } else if (strcmp(arg, "--list") == 0) {
                o->list_only = 1;
            } else if (strcmp(arg, "--keep-sandbox") == 0) {
                o->keep_sandbox = 1;
            } else {
                fprintf(stderr, "Unknown option: %s\n", arg);
                usage(argv[0]);
                return 2;
            }
        } else {
            if (positional == 0) {
                o->project = arg;
                positional++;
            } else if (positional == 1) {
                o->path = arg;
                positional++;
            } else {
                fprintf(stderr, "Too many positional arguments\n");
                usage(argv[0]);
                return 2;
            }
        }
        i++;
    }
    return 0;
}

static int compile_harness(const char *sandbox, const Options *o, char *harness_path, size_t hp_sz) {
    char cmd[4096];
    const char *harness_core = "/Users/macbook/Crafts/prtest-orch/prtest/harness/core";
    const char *harness_libft = "/Users/macbook/Crafts/prtest-orch/prtest/harness/libft";
    
    snprintf(harness_path, hp_sz, "%s/harness", sandbox);
    
    snprintf(cmd, sizeof cmd,
        "cc -std=c11 -O0 -g -Wall -Wextra -Werror "
        "-I%s "
        "%s/*.c %s/t_ctype.c "
        "%s/libft.a -o %s 2>&1",
        harness_core, harness_core, harness_libft, sandbox, harness_path);
    
    if (o->verbose) {
        fprintf(stderr, "Compiling harness: %s\n", cmd);
    }
    
    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;
    
    char buf[1024];
    while (fgets(buf, sizeof buf, fp)) {
        if (o->verbose) fputs(buf, stderr);
    }
    int status = pclose(fp);
    if (status != 0) {
        if (!o->verbose) {
            fprintf(stderr, "Harness compilation failed (use -v for details)\n");
        }
        return -1;
    }
    return 0;
}

int run_harness(const char *sandbox, const Options *o, const ProjectInfo *info) {
    char harness_path[1024];
    
    if (compile_harness(sandbox, o, harness_path, sizeof harness_path) != 0) {
        return -1;
    }
    
    int pipefd[2];
    if (pipe(pipefd) != 0) {
        perror("pipe");
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        if (dup2(pipefd[1], 3) < 0) _exit(2);
        close(pipefd[1]);
        
        char *argv[] = {
            (char *)harness_path,
            o->function ? (char *)o->function : NULL,
            o->verbose ? "-v" : NULL,
            o->strict ? "--strict" : NULL,
            o->probe_ub ? "--probe-ub" : NULL,
            o->stress ? "--stress" : NULL,
            "--timeout", NULL,
            "--seed", NULL,
            NULL
        };
        char timeout_str[32];
        snprintf(timeout_str, sizeof timeout_str, "%.3f", o->timeout_s);
        argv[7] = timeout_str;
        char seed_str[32];
        snprintf(seed_str, sizeof seed_str, "%u", o->seed);
        argv[9] = seed_str;
        
        int argc = 1;
        for (int i = 1; argv[i]; i++) if (argv[i]) argc++;
        char **filtered = malloc((argc + 1) * sizeof(char *));
        int j = 0;
        for (int i = 0; argv[i]; i++) if (argv[i]) filtered[j++] = argv[i];
        filtered[j] = NULL;
        
        execv(harness_path, filtered);
        _exit(127);
    }
    close(pipefd[1]);
    
    results_parse(pipefd[0], o, info);
    close(pipefd[0]);
    
    int status = 0;
    waitpid(pid, &status, 0);
    
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

int main(int argc, char **argv) {
    Options o;
    ProjectInfo info;
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
    
    if (!o.keep_sandbox) {
        char rmcmd[1024];
        snprintf(rmcmd, sizeof rmcmd, "rm -rf %s", sb);
        system(rmcmd);
    } else {
        fprintf(stderr, "Sandbox kept at: %s\n", sb);
    }
    return rc;
}