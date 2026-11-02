#include "../core/protocol.h"
#include "proto.h"
#include "cmp.h"
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

/* ---- ft_memset / ft_bzero ---- */

static void test_memset_zero_length(void)
{
    char buf[11] = "aaaaaaaaaa";
    expect_eq_int("ft_memset n=0 returns s", (long)ft_memset(buf, 'x', 0), (long)buf, __FILE__, __LINE__);
    expect_true("ft_memset n=0 unchanged", strcmp(buf, "aaaaaaaaaa") == 0, __FILE__, __LINE__);
}

static void test_memset_basic_sizes(void)
{
    for (size_t n = 1; n <= 70; n++) {
        unsigned char buf[80];
        memset(buf, 0xAA, sizeof(buf));
        ft_memset(buf, 'z', n);
        for (size_t i = 0; i < n; i++)
            expect_true("ft_memset byte set", buf[i] == 'z', __FILE__, __LINE__);
        for (size_t i = n; i < sizeof(buf); i++)
            expect_true("ft_memset beyond n untouched", buf[i] == 0xAA, __FILE__, __LINE__);
    }
}

static void test_memset_large_size(void)
{
    size_t n = 100000;
    char *buf = malloc(n);
    ft_memset(buf, 'x', n);
    for (size_t i = 0; i < n; i++)
        if (buf[i] != 'x') { expect_true("ft_memset large all bytes", 0, __FILE__, __LINE__); break; }
    free(buf);
}

static void test_memset_alignment_grid(void)
{
    for (size_t align = 0; align < 8; align++) {
        Gbuf g = gbuf_new(64 + align);
        char *p = gbuf_at(g, align);
        ft_memset(p, 'a', 32);
        for (size_t i = 0; i < 32; i++)
            expect_true("ft_memset alignment", p[i] == 'a', __FILE__, __LINE__);
        expect_true("ft_memset canary after", p[32] == 0, __FILE__, __LINE__);
        gbuf_free(g);
    }
}

static void test_memset_value_conversion(void)
{
    char buf1[10], buf2[10];
    ft_memset(buf1, 256, 5);  /* 256 -> 0 */
    ft_memset(buf2, 0, 5);
    expect_mem_eq("ft_memset value 256", buf1, buf2, 5, __FILE__, __LINE__);

    ft_memset(buf1, -1, 5);   /* -1 -> 0xFF */
    memset(buf2, 0xFF, 5);
    expect_mem_eq("ft_memset value -1", buf1, buf2, 5, __FILE__, __LINE__);
}

static void test_memset_return_value(void)
{
    char buf[20];
    void *ret = ft_memset(buf, 'x', 10);
    expect_eq_int("ft_memset returns s", (long)ret, (long)buf, __FILE__, __LINE__);
}

static void test_memset_canaries_guard_page(void)
{
    Gbuf g = gbuf_new(32);
    char *p = gbuf_at(g, 0);
    ft_memset(p, 'x', 16);
    expect_true("ft_memset canary after", p[16] == 0, __FILE__, __LINE__);
    gbuf_free(g);
}

static void test_bzero_zero_length(void)
{
    char buf[11] = "aaaaaaaaaa";
    ft_bzero(buf, 0);
    expect_true("ft_bzero n=0 unchanged", strcmp(buf, "aaaaaaaaaa") == 0, __FILE__, __LINE__);
}

static void test_bzero_basic(void)
{
    unsigned char buf[20];
    memset(buf, 0xFF, sizeof(buf));
    ft_bzero(buf + 2, 10);
    for (size_t i = 0; i < 2; i++)
        expect_true("ft_bzero before untouched", buf[i] == 0xFF, __FILE__, __LINE__);
    for (size_t i = 2; i < 12; i++)
        expect_true("ft_bzero zeroed", buf[i] == 0, __FILE__, __LINE__);
    for (size_t i = 12; i < sizeof(buf); i++)
        expect_true("ft_bzero after untouched", buf[i] == 0xFF, __FILE__, __LINE__);
}

static void test_bzero_large_and_alignment(void)
{
    for (size_t align = 0; align < 8; align++) {
        Gbuf g = gbuf_new(64 + align);
        unsigned char *p = (unsigned char *)gbuf_at(g, align);
        memset(p, 0xAA, 32);
        ft_bzero(p, 16);
        for (size_t i = 0; i < 16; i++)
            expect_true("ft_bzero alignment zeroed", p[i] == 0, __FILE__, __LINE__);
        for (size_t i = 16; i < 32; i++)
            expect_true("ft_bzero alignment untouched", p[i] == 0xAA, __FILE__, __LINE__);
        gbuf_free(g);
    }
}

TEST("ft_memset n=0", test_memset_zero_length);
TEST("ft_memset sizes 1..70", test_memset_basic_sizes);
TEST("ft_memset 100k", test_memset_large_size);
TEST("ft_memset alignment grid", test_memset_alignment_grid);
TEST("ft_memset value conversion", test_memset_value_conversion);
TEST("ft_memset return value", test_memset_return_value);
TEST("ft_memset canaries guard page", test_memset_canaries_guard_page);
TEST("ft_bzero n=0", test_bzero_zero_length);
TEST("ft_bzero basic", test_bzero_basic);
TEST("ft_bzero large and alignment", test_bzero_large_and_alignment);

/* ---- ft_memcpy ---- */

static void test_memcpy_zero_length(void)
{
    char src[11] = "aaaaaaaaaa";
    char dst[11] = "bbbbbbbbbb";
    void *ret = ft_memcpy(dst, src, 0);
    expect_eq_int("ft_memcpy n=0 returns dst", (long)ret, (long)dst, __FILE__, __LINE__);
    expect_true("ft_memcpy n=0 unchanged", strcmp(dst, "bbbbbbbbbb") == 0, __FILE__, __LINE__);
}

static void test_memcpy_alignment_grid(void)
{
    for (size_t src_align = 0; src_align < 8; src_align++) {
        for (size_t dst_align = 0; dst_align < 8; dst_align++) {
            Gbuf gsrc = gbuf_new(64 + src_align);
            Gbuf gdst = gbuf_new(64 + dst_align);
            char *s = gbuf_at(gsrc, src_align);
            char *d = gbuf_at(gdst, dst_align);
            for (size_t i = 0; i < 32; i++) s[i] = (char)(i + 1);
            ft_memcpy(d, s, 32);
            expect_mem_eq("ft_memcpy alignment", d, s, 32, __FILE__, __LINE__);
            expect_eq_int("ft_memcpy returns dst", (long)ft_memcpy(d, s, 32), (long)d, __FILE__, __LINE__);
            gbuf_free(gsrc);
            gbuf_free(gdst);
        }
    }
}

static void test_memcpy_embedded_nulls(void)
{
    char src[] = "ab\0cd\0ef";
    char dst[20];
    ft_memcpy(dst, src, 9);
    expect_mem_eq("ft_memcpy embedded NULs", dst, src, 9, __FILE__, __LINE__);
}

static void test_memcpy_0xff_bytes(void)
{
    unsigned char src[20];
    unsigned char dst[20];
    for (size_t i = 0; i < 20; i++) src[i] = 0xFF;
    ft_memcpy(dst, src, 20);
    expect_mem_eq("ft_memcpy 0xFF bytes", dst, src, 20, __FILE__, __LINE__);
}

static void test_memcpy_1mb(void)
{
    size_t n = 1024 * 1024;
    char *src = malloc(n);
    char *dst = malloc(n);
    for (size_t i = 0; i < n; i++) src[i] = (char)(rng_next() & 0xFF);
    ft_memcpy(dst, src, n);
    expect_mem_eq("ft_memcpy 1MB", dst, src, n, __FILE__, __LINE__);
    free(src);
    free(dst);
}

static void test_memcpy_return_value(void)
{
    char src[20], dst[20];
    void *ret = ft_memcpy(dst, src, 10);
    expect_eq_int("ft_memcpy returns dst", (long)ret, (long)dst, __FILE__, __LINE__);
}

TEST("ft_memcpy n=0", test_memcpy_zero_length);
TEST("ft_memcpy alignment grid", test_memcpy_alignment_grid);
TEST("ft_memcpy embedded NULs", test_memcpy_embedded_nulls);
TEST("ft_memcpy 0xFF bytes", test_memcpy_0xff_bytes);
TEST("ft_memcpy 1MB", test_memcpy_1mb);
TEST("ft_memcpy return value", test_memcpy_return_value);

/* ---- ft_memmove ---- */

static void test_memmove_exhaustive_sweep(void)
{
    for (size_t src_off = 0; src_off < 64; src_off++) {
        for (size_t dst_off = 0; dst_off < 64; dst_off++) {
            for (size_t n = 0; n <= 64 - (src_off > dst_off ? src_off : dst_off); n++) {
                char buf1[128], buf2[128];
                for (size_t i = 0; i < 128; i++) { buf1[i] = (char)(i + 1); buf2[i] = (char)(i + 1); }
                memmove(buf1 + dst_off, buf1 + src_off, n);
                ft_memmove(buf2 + dst_off, buf2 + src_off, n);
                expect_mem_eq("ft_memmove sweep", buf2, buf1, 128, __FILE__, __LINE__);
            }
        }
    }
}

static void test_memmove_forward_overlap(void)
{
    char buf1[20] = "abcdefghij";
    char buf2[20] = "abcdefghij";
    memmove(buf1 + 3, buf1, 5);   /* forward overlap */
    ft_memmove(buf2 + 3, buf2, 5);
    expect_mem_eq("ft_memmove forward overlap", buf2, buf1, 20, __FILE__, __LINE__);
}

static void test_memmove_backward_overlap(void)
{
    char buf1[20] = "abcdefghij";
    char buf2[20] = "abcdefghij";
    memmove(buf1, buf1 + 3, 5);   /* backward overlap */
    ft_memmove(buf2, buf2 + 3, 5);
    expect_mem_eq("ft_memmove backward overlap", buf2, buf1, 20, __FILE__, __LINE__);
}

static void test_memmove_same_pointer(void)
{
    char buf[21] = "abcdefghij";
    ft_memmove(buf, buf, 10);
    expect_mem_eq("ft_memmove same pointer", buf, "abcdefghij", 11, __FILE__, __LINE__);
}

static void test_memmove_zero_length(void)
{
    char buf[11] = "abcdefghij";
    ft_memmove(buf, buf + 5, 0);
    expect_mem_eq("ft_memmove n=0", buf, "abcdefghij", 11, __FILE__, __LINE__);
}

TEST("ft_memmove exhaustive sweep", test_memmove_exhaustive_sweep);
TEST("ft_memmove forward overlap", test_memmove_forward_overlap);
TEST("ft_memmove backward overlap", test_memmove_backward_overlap);
TEST("ft_memmove same pointer", test_memmove_same_pointer);
TEST("ft_memmove n=0", test_memmove_zero_length);

/* ---- ft_memchr ---- */

static void test_memchr_zero_length(void)
{
    char buf[11] = "abcdefghij";
    expect_true("ft_memchr n=0", ft_memchr(buf, 'a', 0) == NULL, __FILE__, __LINE__);
}

static void test_memchr_embedded_nulls(void)
{
    char buf[] = "ab\0cd\0ef";
    void *p = ft_memchr(buf, '\0', 9);
    expect_true("ft_memchr finds first NUL", p != NULL, __FILE__, __LINE__);
    expect_eq_int("ft_memchr first NUL pos", (char *)p - buf, 2, __FILE__, __LINE__);
}

static void test_memchr_unsigned_compare(void)
{
    unsigned char buf[] = {0x80, 0xFF, 0x7F, 0x00};
    void *p = ft_memchr(buf, (int)0xFF, 4);
    expect_true("ft_memchr unsigned 0xFF", p != NULL, __FILE__, __LINE__);
    expect_eq_int("ft_memchr 0xFF pos", (unsigned char *)p - buf, 1, __FILE__, __LINE__);
}

static void test_memchr_hit_at_n_minus_1(void)
{
    char buf[] = "abcde";
    void *p = ft_memchr(buf, 'e', 5);
    expect_true("ft_memchr hit at n-1", p != NULL, __FILE__, __LINE__);
    expect_eq_int("ft_memchr hit at n-1 pos", (char *)p - buf, 4, __FILE__, __LINE__);

    p = ft_memchr(buf, 'e', 4);
    expect_true("ft_memchr miss at n-1", p == NULL, __FILE__, __LINE__);
}

static void test_memchr_guard_after_n(void)
{
    Gbuf g = gbuf_new(32);
    char *p = gbuf_at(g, 0);
    memset(p, 'x', 16);
    p[16] = 'y';
    void *res = ft_memchr(p, 'y', 16);
    expect_true("ft_memchr no overread guard", res == NULL, __FILE__, __LINE__);
    gbuf_free(g);
}

static void test_memchr_c_converted_to_uchar(void)
{
    char buf[] = {'a', (char)0xFF, 'b', '\0'};
    void *p = ft_memchr(buf, 255, 4);
    expect_true("ft_memchr c=255", p != NULL, __FILE__, __LINE__);
    expect_eq_int("ft_memchr c=255 pos", (char *)p - buf, 1, __FILE__, __LINE__);

    p = ft_memchr(buf, -1, 4);
    expect_true("ft_memchr c=-1", p != NULL, __FILE__, __LINE__);
    expect_eq_int("ft_memchr c=-1 pos", (char *)p - buf, 1, __FILE__, __LINE__);
}

static void test_memchr_vs_ref(void)
{
    unsigned char data[] = {0x00, 0xFF, 0x80, 0x7F, 0x00, 0xAA, 0x55, 0x00};
    for (size_t n = 0; n <= 8; n++) {
        for (int c = -128; c <= 255; c++) {
            void *got = ft_memchr(data, c, n);
            void *want = memchr(data, c, n);
            char expr[64];
            snprintf(expr, sizeof(expr), "ft_memchr vs libc n=%zu c=%d", n, c);
            if (want == NULL)
                expect_true(expr, got == NULL, __FILE__, __LINE__);
            else
                expect_eq_int(expr, (char *)got - (char *)data, (char *)want - (char *)data, __FILE__, __LINE__);
        }
    }
}

TEST("ft_memchr n=0", test_memchr_zero_length);
TEST("ft_memchr embedded NULs", test_memchr_embedded_nulls);
TEST("ft_memchr unsigned compare", test_memchr_unsigned_compare);
TEST("ft_memchr hit at n-1", test_memchr_hit_at_n_minus_1);
TEST("ft_memchr guard after n", test_memchr_guard_after_n);
TEST("ft_memchr c converted to uchar", test_memchr_c_converted_to_uchar);
TEST("ft_memchr vs libc", test_memchr_vs_ref);

/* ---- ft_memcmp ---- */

static void test_memcmp_zero_length(void)
{
    expect_eq_int("ft_memcmp n=0", ft_memcmp("a", "b", 0), 0, __FILE__, __LINE__);
}

static void test_memcmp_embedded_nulls(void)
{
    char s1[] = "ab\0cd";
    char s2[] = "ab\0ce";
    expect_true("ft_memcmp embedded NUL diff", ft_memcmp(s1, s2, 6) < 0, __FILE__, __LINE__);
    expect_eq_int("ft_memcmp sign match ref", 
        (ft_memcmp(s1, s2, 6) > 0) - (ft_memcmp(s1, s2, 6) < 0),
        (ref_memcmp_sign(s1, s2, 6) > 0) - (ref_memcmp_sign(s1, s2, 6) < 0),
        __FILE__, __LINE__);
}

static void test_memcmp_unsigned_semantics(void)
{
    unsigned char s1[] = {0xFF, 0x00};
    unsigned char s2[] = {0x7F, 0x00};
    expect_true("ft_memcmp unsigned 0xFF > 0x7F", ft_memcmp(s1, s2, 2) > 0, __FILE__, __LINE__);
    expect_eq_int("ft_memcmp sign match ref",
        (ft_memcmp(s1, s2, 2) > 0) - (ft_memcmp(s1, s2, 2) < 0),
        (ref_memcmp_sign(s1, s2, 2) > 0) - (ref_memcmp_sign(s1, s2, 2) < 0),
        __FILE__, __LINE__);
}

static void test_memcmp_hit_at_n_minus_1(void)
{
    char s1[] = "abcde";
    char s2[] = "abcdf";
    expect_true("ft_memcmp diff at n-1", ft_memcmp(s1, s2, 5) < 0, __FILE__, __LINE__);
    expect_eq_int("ft_memcmp equal at n-1", ft_memcmp(s1, s2, 4), 0, __FILE__, __LINE__);
}

static void test_memcmp_guard_after_n(void)
{
    Gbuf g = gbuf_new(32);
    char *p = gbuf_at(g, 0);
    memset(p, 'x', 16);
    p[16] = 'y';
    expect_eq_int("ft_memcmp no overread guard", ft_memcmp(p, "xxxxxxxxxxxxxxxx", 16), 0, __FILE__, __LINE__);
    gbuf_free(g);
}

static void test_memcmp_vs_ref_sign(void)
{
    unsigned char data1[] = {0x00, 0xFF, 0x80, 0x7F, 0x00, 0xAA, 0x55, 0x00};
    unsigned char data2[] = {0x00, 0xFE, 0x80, 0x7F, 0x01, 0xAA, 0x55, 0xFF};
    for (size_t n = 0; n <= 8; n++) {
        int got = ft_memcmp(data1, data2, n);
        int want = ref_memcmp_sign(data1, data2, n);
        int got_sign = (got > 0) - (got < 0);
        int want_sign = (want > 0) - (want < 0);
        char expr[64];
        snprintf(expr, sizeof(expr), "ft_memcmp sign vs ref n=%zu", n);
        expect_eq_int(expr, got_sign, want_sign, __FILE__, __LINE__);
    }
}

TEST("ft_memcmp n=0", test_memcmp_zero_length);
TEST("ft_memcmp embedded NULs", test_memcmp_embedded_nulls);
TEST("ft_memcmp unsigned semantics", test_memcmp_unsigned_semantics);
TEST("ft_memcmp hit at n-1", test_memcmp_hit_at_n_minus_1);
TEST("ft_memcmp guard after n", test_memcmp_guard_after_n);
TEST("ft_memcmp sign vs ref", test_memcmp_vs_ref_sign);

/* ---- ft_calloc ---- */

static void test_calloc_zeroes_memory(void)
{
    awrap_reset();
    void *p = ft_calloc(100, 1);
    expect_true("ft_calloc all zero", p != NULL, __FILE__, __LINE__);
    for (size_t i = 0; i < 100; i++)
        expect_true("ft_calloc byte zero", ((char *)p)[i] == 0, __FILE__, __LINE__);
    free(p);
}

static void test_calloc_zero_size(void)
{
    void *p1 = ft_calloc(0, 10);
    void *p2 = ft_calloc(10, 0);
    void *p3 = ft_calloc(0, 0);
    expect_true("ft_calloc(0,10) non-NULL", p1 != NULL, __FILE__, __LINE__);
    expect_true("ft_calloc(10,0) non-NULL", p2 != NULL, __FILE__, __LINE__);
    expect_true("ft_calloc(0,0) non-NULL", p3 != NULL, __FILE__, __LINE__);
    expect_true("ft_calloc distinct pointers", p1 != p2 && p2 != p3 && p1 != p3, __FILE__, __LINE__);
    free(p1); free(p2); free(p3);
}

static void test_calloc_overflow_size_t(void)
{
    void *p = ft_calloc(SIZE_MAX / 2 + 1, 2);
    expect_true("ft_calloc overflow returns NULL", p == NULL, __FILE__, __LINE__);
}

static void test_calloc_large_size(void)
{
    void *p = ft_calloc(100000, 10);
    expect_true("ft_calloc large", p != NULL, __FILE__, __LINE__);
    if (p) {
        for (size_t i = 0; i < 1000000; i += 1000)
            expect_true("ft_calloc large zeroed", ((char *)p)[i] == 0, __FILE__, __LINE__);
        free(p);
    }
}

/* Tier B: failure injection - deferred due to harness awrap_fail_at limitation */
/* static void test_calloc_failure_injection(void) { ... } */

TEST("ft_calloc zeroes memory", test_calloc_zeroes_memory);
TEST("ft_calloc zero size args", test_calloc_zero_size);
TEST("ft_calloc overflow", test_calloc_overflow_size_t);
TEST("ft_calloc large size", test_calloc_large_size);

/* ---- ft_strdup ---- */

static void test_strdup_independence(void)
{
    char *orig = strdup("hello world");
    char *copy = ft_strdup(orig);
    strcpy(orig, "modified");
    expect_true("ft_strdup independent", strcmp(copy, "hello world") == 0, __FILE__, __LINE__);
    free(orig);
    free(copy);
}

static void test_strdup_distinct_pointer(void)
{
    char *s = "test";
    char *copy = ft_strdup(s);
    expect_true("ft_strdup distinct pointer", copy != s, __FILE__, __LINE__);
    free(copy);
}

static void test_strdup_empty_string(void)
{
    char *copy = ft_strdup("");
    expect_true("ft_strdup empty non-NULL", copy != NULL, __FILE__, __LINE__);
    expect_eq_int("ft_strdup empty content", strcmp(copy, ""), 0, __FILE__, __LINE__);
    free(copy);
}

static void test_strdup_long_string(void)
{
    char long_str[10000];
    for (size_t i = 0; i < 9999; i++) long_str[i] = 'a' + (i % 26);
    long_str[9999] = '\0';
    char *copy = ft_strdup(long_str);
    expect_true("ft_strdup long string", strcmp(copy, long_str) == 0, __FILE__, __LINE__);
    free(copy);
}

static void test_strdup_vs_libc(void)
{
    const char *tests[] = {"", "a", "hello", "longer string with spaces", "\xFF\xFE\x00"};
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        char *got = ft_strdup(tests[i]);
        char *want = strdup(tests[i]);
        expect_true("ft_strdup vs libc non-NULL", got != NULL && want != NULL, __FILE__, __LINE__);
        expect_mem_eq("ft_strdup vs libc content", got, want, strlen(want) + 1, __FILE__, __LINE__);
        free(got);
        free(want);
    }
}

TEST("ft_strdup independence", test_strdup_independence);
TEST("ft_strdup distinct pointer", test_strdup_distinct_pointer);
TEST("ft_strdup empty string", test_strdup_empty_string);
TEST("ft_strdup long string", test_strdup_long_string);
TEST("ft_strdup vs libc", test_strdup_vs_libc);