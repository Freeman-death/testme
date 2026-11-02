#include "prtest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

static int copy_file(const char *src, const char *dst) {
    int in = open(src, O_RDONLY);
    if (in < 0) return -1;
    struct stat st;
    fstat(in, &st);
    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode & 0777);
    if (out < 0) { close(in); return -1; }
    char buf[8192];
    ssize_t r;
    while ((r = read(in, buf, sizeof buf)) > 0) {
        ssize_t w = write(out, buf, r);
        if (w != r) { close(in); close(out); return -1; }
    }
    close(in);
    close(out);
    return r < 0 ? -1 : 0;
}

static int copy_symlink(const char *src, const char *dst) {
    char target[1024];
    ssize_t len = readlink(src, target, sizeof target - 1);
    if (len < 0) return -1;
    target[len] = '\0';
    return symlink(target, dst) == 0 ? 0 : -1;
}

static int copy_recursive(const char *src, const char *dst) {
    struct stat st;
    if (lstat(src, &st) < 0) return -1;
    
    if (S_ISDIR(st.st_mode)) {
        if (mkdir(dst, st.st_mode & 0777) < 0 && errno != EEXIST) return -1;
        DIR *d = opendir(src);
        if (!d) return -1;
        struct dirent *ent;
        while ((ent = readdir(d)) != NULL) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
            char srcp[1024], dstp[1024];
            snprintf(srcp, sizeof srcp, "%s/%s", src, ent->d_name);
            snprintf(dstp, sizeof dstp, "%s/%s", dst, ent->d_name);
            if (copy_recursive(srcp, dstp) < 0) {
                closedir(d);
                return -1;
            }
        }
        closedir(d);
    } else if (S_ISLNK(st.st_mode)) {
        if (copy_symlink(src, dst) < 0) return -1;
    } else if (S_ISREG(st.st_mode)) {
        if (copy_file(src, dst) < 0) return -1;
    }
    return 0;
}

int sandbox_copy(const char *src, char *dst, size_t dstsz) {
    snprintf(dst, dstsz, "/tmp/prtest-sb-%d", (int)getpid());
    return copy_recursive(src, dst);
}

static int run_make_in_sandbox(const char *sandbox, char *output, size_t outsz) {
    char cmd[512];
    snprintf(cmd, sizeof cmd, "cd %s && make 2>&1", sandbox);
    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;
    size_t total = 0;
    char buf[1024];
    while (fgets(buf, sizeof buf, fp)) {
        size_t len = strlen(buf);
        if (total + len < outsz - 1) {
            memcpy(output + total, buf, len);
            total += len;
        }
    }
    output[total] = '\0';
    return pclose(fp);
}

int build_sandbox(const char *sandbox, const Options *o) {
    char output[16384];
    int status = run_make_in_sandbox(sandbox, output, sizeof output);
    if (o->verbose) {
        fprintf(stderr, "%s", output);
    }
    if (status != 0) {
        if (!o->verbose) fprintf(stderr, "%s", output);
        return -1;
    }
    char libpath[1024];
    snprintf(libpath, sizeof libpath, "%s/libft.a", sandbox);
    struct stat st;
    if (stat(libpath, &st) != 0) {
        fprintf(stderr, "libft.a not found after build\n");
        return -1;
    }
    return 0;
}