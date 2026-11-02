#include "../core/protocol.h"
#include "proto.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

/* Helper: create temp file and return fd */
static int make_temp_fd(void) {
    char template[] = "/tmp/prtest_XXXXXX";
    return mkstemp(template);
}

/* Helper: read all data from fd, return length via out_len */
static char *read_all(int fd, size_t *out_len) {
    off_t cur = lseek(fd, 0, SEEK_CUR);
    lseek(fd, 0, SEEK_SET);
    char buf[4096];
    size_t cap = 4096, len = 0;
    char *data = malloc(cap);
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        if (len + n >= cap) {
            cap *= 2;
            data = realloc(data, cap);
        }
        memcpy(data + len, buf, n);
        len += n;
    }
    lseek(fd, cur, SEEK_SET);
    if (out_len) *out_len = len;
    return data;
}

static void check_fd_output(int fd, const char *expected, size_t expected_len, const char *test_name) {
    size_t got_len;
    char *got = read_all(fd, &got_len);
    int ok = 1;
    if (!got) ok = 0;
    else if (got_len != expected_len) ok = 0;
    else if (memcmp(got, expected, expected_len) != 0) ok = 0;
    expect_true(test_name, ok, __FILE__, __LINE__);
    free(got);
}

/* ===== ft_putchar_fd tests ===== */

static void test_putchar_fd_null(void) {
    int fd = make_temp_fd();
    ft_putchar_fd('\0', fd);
    check_fd_output(fd, "\0", 1, "ft_putchar_fd writes \\0 as 0x00");
    close(fd);
}

static void test_putchar_fd_0xff(void) {
    int fd = make_temp_fd();
    ft_putchar_fd((char)0xFF, fd);
    unsigned char expected = 0xFF;
    check_fd_output(fd, (char*)&expected, 1, "ft_putchar_fd writes 0xFF");
    close(fd);
}

static void test_putchar_fd_stdout_capture(void) {
    int saved = dup(1);
    int fd = make_temp_fd();
    dup2(fd, 1);
    ft_putchar_fd('x', 1);
    fflush(stdout);
    dup2(saved, 1);
    close(saved);
    check_fd_output(fd, "x", 1, "ft_putchar_fd fd=1 writes to stdout");
    close(fd);
}

static void test_putchar_fd_stderr_capture(void) {
    int saved = dup(2);
    int fd = make_temp_fd();
    dup2(fd, 2);
    ft_putchar_fd('y', 2);
    fflush(stderr);
    dup2(saved, 2);
    close(saved);
    check_fd_output(fd, "y", 1, "ft_putchar_fd fd=2 writes to stderr");
    close(fd);
}

static void test_putchar_fd_invalid_no_crash(void) {
    int saved_out = dup(1);
    int saved_err = dup(2);
    int out_fd = make_temp_fd();
    int err_fd = make_temp_fd();
    dup2(out_fd, 1);
    dup2(err_fd, 2);
    ft_putchar_fd('z', 999);
    fflush(stdout);
    fflush(stderr);
    dup2(saved_out, 1);
    dup2(saved_err, 2);
    close(saved_out);
    close(saved_err);
    check_fd_output(out_fd, "", 0, "ft_putchar_fd invalid fd doesn't write to stdout");
    check_fd_output(err_fd, "", 0, "ft_putchar_fd invalid fd doesn't write to stderr");
    close(out_fd);
    close(err_fd);
}

TEST("ft_putchar_fd null byte", test_putchar_fd_null);
TEST("ft_putchar_fd 0xFF", test_putchar_fd_0xff);
TEST("ft_putchar_fd stdout capture", test_putchar_fd_stdout_capture);
TEST("ft_putchar_fd stderr capture", test_putchar_fd_stderr_capture);
TEST("ft_putchar_fd invalid fd no crash/leak", test_putchar_fd_invalid_no_crash);

/* ===== ft_putstr_fd tests ===== */

static void test_putstr_fd_empty(void) {
    int fd = make_temp_fd();
    ft_putstr_fd("", fd);
    check_fd_output(fd, "", 0, "ft_putstr_fd empty string writes nothing");
    close(fd);
}

static void test_putstr_fd_normal(void) {
    int fd = make_temp_fd();
    ft_putstr_fd("hello", fd);
    check_fd_output(fd, "hello", 5, "ft_putstr_fd normal string");
    close(fd);
}

static void test_putstr_fd_1mb(void) {
    int fd = make_temp_fd();
    size_t n = 1024 * 1024;
    char *s = malloc(n + 1);
    memset(s, 'a', n);
    s[n] = '\0';
    ft_putstr_fd(s, fd);
    char *got = read_all(fd, NULL);
    expect_true("ft_putstr_fd 1MB exact length", got && strlen(got) == n, __FILE__, __LINE__);
    for (size_t i = 0; i < n; i++) {
        if (got[i] != 'a') { expect_true("ft_putstr_fd 1MB content", 0, __FILE__, __LINE__); break; }
    }
    free(got);
    free(s);
    close(fd);
}

static void test_putstr_fd_stdout_capture(void) {
    int saved = dup(1);
    int fd = make_temp_fd();
    dup2(fd, 1);
    ft_putstr_fd("stdout", 1);
    fflush(stdout);
    dup2(saved, 1);
    close(saved);
    check_fd_output(fd, "stdout", 6, "ft_putstr_fd fd=1 writes to stdout");
    close(fd);
}

static void test_putstr_fd_invalid_no_leak(void) {
    int saved_out = dup(1);
    int out_fd = make_temp_fd();
    dup2(out_fd, 1);
    ft_putstr_fd("leak?", 999);
    fflush(stdout);
    dup2(saved_out, 1);
    close(saved_out);
    check_fd_output(out_fd, "", 0, "ft_putstr_fd invalid fd no stdout leak");
    close(out_fd);
}

/* Tier C: ft_putstr_fd(NULL, ...) — do NOT run by default. Commented out.
static void test_putstr_fd_null_tier_c(void) {
    int fd = make_temp_fd();
    ft_putstr_fd(NULL, fd);  // UB, may crash
    close(fd);
}
*/

TEST("ft_putstr_fd empty", test_putstr_fd_empty);
TEST("ft_putstr_fd normal", test_putstr_fd_normal);
TEST("ft_putstr_fd 1MB", test_putstr_fd_1mb);
TEST("ft_putstr_fd stdout capture", test_putstr_fd_stdout_capture);
TEST("ft_putstr_fd invalid fd no leak", test_putstr_fd_invalid_no_leak);

/* ===== ft_putendl_fd tests ===== */

static void test_putendl_fd_empty(void) {
    int fd = make_temp_fd();
    ft_putendl_fd("", fd);
    check_fd_output(fd, "\n", 1, "ft_putendl_fd empty writes newline");
    close(fd);
}

static void test_putendl_fd_normal(void) {
    int fd = make_temp_fd();
    ft_putendl_fd("x", fd);
    check_fd_output(fd, "x\n", 2, "ft_putendl_fd writes string + newline");
    close(fd);
}

static void test_putendl_fd_stdout_capture(void) {
    int saved = dup(1);
    int fd = make_temp_fd();
    dup2(fd, 1);
    ft_putendl_fd("line", 1);
    fflush(stdout);
    dup2(saved, 1);
    close(saved);
    check_fd_output(fd, "line\n", 5, "ft_putendl_fd fd=1 writes to stdout");
    close(fd);
}

TEST("ft_putendl_fd empty", test_putendl_fd_empty);
TEST("ft_putendl_fd normal", test_putendl_fd_normal);
TEST("ft_putendl_fd stdout capture", test_putendl_fd_stdout_capture);

/* ===== ft_putnbr_fd tests ===== */

static void test_putnbr_fd_zero(void) {
    int fd = make_temp_fd();
    ft_putnbr_fd(0, fd);
    check_fd_output(fd, "0", 1, "ft_putnbr_fd zero");
    close(fd);
}

static void test_putnbr_fd_positive(void) {
    int fd = make_temp_fd();
    ft_putnbr_fd(42, fd);
    check_fd_output(fd, "42", 2, "ft_putnbr_fd positive");
    close(fd);
}

static void test_putnbr_fd_negative(void) {
    int fd = make_temp_fd();
    ft_putnbr_fd(-42, fd);
    check_fd_output(fd, "-42", 3, "ft_putnbr_fd negative");
    close(fd);
}

static void test_putnbr_fd_int_min(void) {
    int fd = make_temp_fd();
    ft_putnbr_fd(INT_MIN, fd);
    char expected[16];
    snprintf(expected, sizeof(expected), "%d", INT_MIN);
    check_fd_output(fd, expected, strlen(expected), "ft_putnbr_fd INT_MIN matches snprintf");
    close(fd);
}

static void test_putnbr_fd_int_max(void) {
    int fd = make_temp_fd();
    ft_putnbr_fd(INT_MAX, fd);
    char expected[16];
    snprintf(expected, sizeof(expected), "%d", INT_MAX);
    check_fd_output(fd, expected, strlen(expected), "ft_putnbr_fd INT_MAX matches snprintf");
    close(fd);
}

static void test_putnbr_fd_stdout_capture(void) {
    int saved = dup(1);
    int fd = make_temp_fd();
    dup2(fd, 1);
    ft_putnbr_fd(123, 1);
    fflush(stdout);
    dup2(saved, 1);
    close(saved);
    check_fd_output(fd, "123", 3, "ft_putnbr_fd fd=1 writes to stdout");
    close(fd);
}

static void test_putnbr_fd_invalid_no_crash(void) {
    int saved_out = dup(1);
    int out_fd = make_temp_fd();
    dup2(out_fd, 1);
    ft_putnbr_fd(999, 999);
    fflush(stdout);
    dup2(saved_out, 1);
    close(saved_out);
    check_fd_output(out_fd, "", 0, "ft_putnbr_fd invalid fd no stdout leak");
    close(out_fd);
}

TEST("ft_putnbr_fd zero", test_putnbr_fd_zero);
TEST("ft_putnbr_fd positive", test_putnbr_fd_positive);
TEST("ft_putnbr_fd negative", test_putnbr_fd_negative);
TEST("ft_putnbr_fd INT_MIN", test_putnbr_fd_int_min);
TEST("ft_putnbr_fd INT_MAX", test_putnbr_fd_int_max);
TEST("ft_putnbr_fd stdout capture", test_putnbr_fd_stdout_capture);
TEST("ft_putnbr_fd invalid fd no crash", test_putnbr_fd_invalid_no_crash);