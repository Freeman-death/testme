#include "prtest.h"
#include <stdio.h>
#include <string.h>

int detect_project(const char *path, ProjectInfo *info) {
    info->name = "libft";
    info->has_makefile = 0;
    info->has_header = 0;
    info->n_ft_files = 0;
    info->status = DETECT_FAIL;
    info->reason = "no Makefile / libft.h found";

    char mk[1024]; snprintf(mk, sizeof mk, "%s/Makefile", path);
    FILE *f = fopen(mk, "r");
    if (f) { info->has_makefile = 1; fclose(f); }
    char h[1024]; snprintf(h, sizeof h, "%s/libft.h", path);
    f = fopen(h, "r");
    if (f) { info->has_header = 1; fclose(f); }

    if (info->has_makefile && info->has_header) {
        info->status = DETECT_OK;
        info->reason = "Makefile + libft.h present";
        return 0;
    }
    return -1;
}
