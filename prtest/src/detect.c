#include "prtest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static int count_ft_files(const char *path) {
    DIR *d = opendir(path);
    if (!d) return 0;
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strncmp(ent->d_name, "ft_", 3) == 0) {
            const char *ext = strrchr(ent->d_name, '.');
            if (ext && strcmp(ext, ".c") == 0) count++;
        }
    }
    closedir(d);
    return count;
}

static int check_makefile_name(const char *path) {
    char mk[1024];
    snprintf(mk, sizeof mk, "%s/Makefile", path);
    FILE *f = fopen(mk, "r");
    if (!f) return 0;
    char line[512];
    int found = 0;
    while (fgets(line, sizeof line, f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (strncmp(p, "NAME", 4) == 0) {
            p += 4;
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '=' || *p == ' ') {
                p++;
                while (*p == ' ' || *p == '\t') p++;
                if (strncmp(p, "libft.a", 7) == 0) {
                    found = 1;
                    break;
                }
            }
        }
    }
    fclose(f);
    return found;
}

static int check_nm_symbol(const char *path) {
    char cmd[512];
    snprintf(cmd, sizeof cmd, "nm -g --defined-only %s/libft.a 2>/dev/null | grep -q '^.* ft_strlen$'", path);
    return system(cmd) == 0;
}

int detect_project(const char *path, ProjectInfo *info) {
    info->name = "libft";
    info->has_makefile = 0;
    info->has_header = 0;
    info->n_ft_files = 0;
    info->status = DETECT_FAIL;
    info->reason = "no Makefile / libft.h found";

    char mk[1024], hdr[1024];
    snprintf(mk, sizeof mk, "%s/Makefile", path);
    snprintf(hdr, sizeof hdr, "%s/libft.h", path);

    FILE *f = fopen(mk, "r");
    if (f) { info->has_makefile = 1; fclose(f); }
    f = fopen(hdr, "r");
    if (f) { info->has_header = 1; fclose(f); }

    info->n_ft_files = count_ft_files(path);

    int score = 0;
    if (info->has_makefile) score += 2;
    if (info->has_header) score += 2;
    if (info->n_ft_files > 0) score += info->n_ft_files > 5 ? 3 : info->n_ft_files;
    if (check_makefile_name(path)) score += 2;
    if (check_nm_symbol(path)) score += 3;

    if (score >= 7) {
        info->status = DETECT_OK;
        info->reason = "Strong libft match (Makefile NAME=libft.a, header, ft_*.c files, ft_strlen symbol)";
    } else if (score >= 4) {
        info->status = DETECT_WARN;
        info->reason = "Possible libft project (partial match)";
    } else {
        info->status = DETECT_FAIL;
        if (!info->has_makefile && !info->has_header) {
            info->reason = "No Makefile or libft.h found";
        } else if (!info->has_makefile) {
            info->reason = "No Makefile found";
        } else if (!info->has_header) {
            info->reason = "No libft.h found";
        } else {
            info->reason = "Weak match (missing NAME=libft.a or ft_strlen symbol)";
        }
    }
    return info->status == DETECT_OK ? 0 : -1;
}