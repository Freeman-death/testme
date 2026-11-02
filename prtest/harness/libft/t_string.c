#include "../core/protocol.h"
#include "proto.h"
#include "strlcpy.h"
#include "strlcat.h"
#include "strnstr.h"
#include "cmp.h"
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

/* ---- ft_strlen ---- */

static void test_strlen_empty(void)
{
    expect_eq_int("ft_strlen(\"\")", ft_strlen(""), 0, __FILE__, __LINE__);
}

static void test_strlen_basic(void)
{
    expect_eq_int("ft_strlen(\"a\")", ft_strlen("a"), 1, __FILE__, __LINE__);
    expect_eq_int("ft_strlen(\"hello\")", ft_strlen("hello"), 5, __FILE__, __LINE__);
}

static void test_strlen_alignment_edges(void)
{
    for (size_t len = 0; len <= 300; len++) {
        Gbuf g = gbuf_new(len + 1);
        for (size_t i = 0; i < len; i++)
            *gbuf_at(g, i) = 'a';
        *gbuf_at(g, len) = '\0';
        expect_eq_int("ft_strlen alignment", ft_strlen(gbuf_at(g, 0)), (long)len, __FILE__, __LINE__);
        gbuf_free(g);
    }
}

static void test_strlen_page_boundaries(void)
{
    size_t lens[] = {4095, 4096, 4097, 1024 * 1024};
    for (size_t i = 0; i < sizeof(lens)/sizeof(lens[0]); i++) {
        size_t len = lens[i];
        Gbuf g = gbuf_new(len + 1);
        for (size_t j = 0; j < len; j++)
            *gbuf_at(g, j) = 'x';
        *gbuf_at(g, len) = '\0';
        expect_eq_int("ft_strlen page boundary", ft_strlen(gbuf_at(g, 0)), (long)len, __FILE__, __LINE__);
        gbuf_free(g);
    }
}

static void test_strlen_high_bit_bytes(void)
{
    char s[] = {0x80, 0xFF, 0xFE, 0x7F, '\0'};
    expect_eq_int("ft_strlen high-bit", ft_strlen(s), 4, __FILE__, __LINE__);
}

static void test_strlen_offset_guard(void)
{
    Gbuf g = gbuf_new(100);
    for (size_t i = 0; i < 50; i++)
        *gbuf_at(g, i) = 'b';
    *gbuf_at(g, 50) = '\0';
    expect_eq_int("ft_strlen offset 10", ft_strlen(gbuf_at(g, 10)), 40, __FILE__, __LINE__);
    expect_eq_int("ft_strlen offset 49", ft_strlen(gbuf_at(g, 49)), 1, __FILE__, __LINE__);
    gbuf_free(g);
}

TEST("ft_strlen empty", test_strlen_empty);
TEST("ft_strlen basic", test_strlen_basic);
TEST("ft_strlen alignment edges 0..300", test_strlen_alignment_edges);
TEST("ft_strlen page boundaries", test_strlen_page_boundaries);
TEST("ft_strlen high-bit bytes", test_strlen_high_bit_bytes);
TEST("ft_strlen offset with guard", test_strlen_offset_guard);

/* ---- ft_strchr / ft_strrchr ---- */

static void test_strchr_first_last_match(void)
{
    const char *s = "hello world";
    expect_ptr_in("ft_strchr first 'l'", ft_strchr(s, 'l'), s, s + strlen(s) + 1, __FILE__, __LINE__);
    expect_eq_int("ft_strchr first 'l' pos", ft_strchr(s, 'l') - s, 2, __FILE__, __LINE__);
    expect_ptr_in("ft_strrchr last 'l'", ft_strrchr(s, 'l'), s, s + strlen(s) + 1, __FILE__, __LINE__);
    expect_eq_int("ft_strrchr last 'l' pos", ft_strrchr(s, 'l') - s, 9, __FILE__, __LINE__);
}

static void test_strchr_terminator(void)
{
    const char *s = "abc";
    expect_ptr_in("ft_strchr '\\0'", ft_strchr(s, '\0'), s, s + 4, __FILE__, __LINE__);
    expect_eq_int("ft_strchr '\\0' pos", ft_strchr(s, '\0') - s, 3, __FILE__, __LINE__);
    expect_ptr_in("ft_strrchr '\\0'", ft_strrchr(s, '\0'), s, s + 4, __FILE__, __LINE__);
    expect_eq_int("ft_strrchr '\\0' pos", ft_strrchr(s, '\0') - s, 3, __FILE__, __LINE__);
}

static void test_strchr_not_found(void)
{
    const char *s = "hello";
    expect_true("ft_strchr not found", ft_strchr(s, 'z') == NULL, __FILE__, __LINE__);
    expect_true("ft_strrchr not found", ft_strrchr(s, 'z') == NULL, __FILE__, __LINE__);
}

static void test_strchr_empty_string(void)
{
    const char *s = "";
    expect_true("ft_strchr empty not found", ft_strchr(s, 'a') == NULL, __FILE__, __LINE__);
    expect_true("ft_strrchr empty not found", ft_strrchr(s, 'a') == NULL, __FILE__, __LINE__);
    expect_eq_int("ft_strchr empty '\\0'", ft_strchr(s, '\0') - s, 0, __FILE__, __LINE__);
}

static void test_strchr_converted_char(void)
{
    char s[] = {'a', (char)0xFF, 'b', '\0'};
    expect_ptr_in("ft_strchr 255", ft_strchr(s, 255), s, s + 4, __FILE__, __LINE__);
    expect_eq_int("ft_strchr 255 pos", ft_strchr(s, 255) - s, 1, __FILE__, __LINE__);
    expect_ptr_in("ft_strchr -1", ft_strchr(s, -1), s, s + 4, __FILE__, __LINE__);
    expect_eq_int("ft_strchr -1 pos", ft_strchr(s, -1) - s, 1, __FILE__, __LINE__);
    expect_ptr_in("ft_strchr 256 wraps to NUL", ft_strchr(s, 256), s, s + 4, __FILE__, __LINE__);
    expect_eq_int("ft_strchr 256 pos", ft_strchr(s, 256) - s, 3, __FILE__, __LINE__);
}

TEST("ft_strchr first/last match", test_strchr_first_last_match);
TEST("ft_strchr/ft_strrchr terminator", test_strchr_terminator);
TEST("ft_strchr/ft_strrchr not found", test_strchr_not_found);
TEST("ft_strchr/ft_strrchr empty string", test_strchr_empty_string);
TEST("ft_strchr converted char", test_strchr_converted_char);

/* ---- ft_strncmp ---- */

static void test_strncmp_n_zero(void)
{
    expect_eq_int("ft_strncmp n=0", ft_strncmp("a", "b", 0), 0, __FILE__, __LINE__);
}

static void test_strncmp_stops_at_nul(void)
{
    Gbuf g = gbuf_new(10);
    *gbuf_at(g, 0) = 'a'; *gbuf_at(g, 1) = '\0'; *gbuf_at(g, 2) = 'x';
    expect_eq_int("ft_strncmp stops at NUL", ft_strncmp(gbuf_at(g, 0), "a", 5), 0, __FILE__, __LINE__);
    gbuf_free(g);
}

static void test_strncmp_difference_beyond_n(void)
{
    expect_eq_int("ft_strncmp diff beyond n", ft_strncmp("abc", "abd", 2), 0, __FILE__, __LINE__);
    expect_true("ft_strncmp diff within n", ft_strncmp("abc", "abd", 3) < 0, __FILE__, __LINE__);
}

static void test_strncmp_unsigned_semantics(void)
{
    char s1[] = {(char)0xFF, '\0'};
    char s2[] = {(char)0x7F, '\0'};
    expect_true("ft_strncmp unsigned 0xFF > 0x7F", ft_strncmp(s1, s2, 2) > 0, __FILE__, __LINE__);
    expect_eq_int("ft_strncmp unsigned sign match ref",
        (ft_strncmp(s1, s2, 2) > 0) - (ft_strncmp(s1, s2, 2) < 0),
        (ref_strncmp_sign(s1, s2, 2) > 0) - (ref_strncmp_sign(s1, s2, 2) < 0),
        __FILE__, __LINE__);
}

static void test_strncmp_prefix_cases(void)
{
    expect_eq_int("ft_strncmp equal prefix", ft_strncmp("abc", "abcdef", 3), 0, __FILE__, __LINE__);
    expect_true("ft_strncmp shorter < longer", ft_strncmp("abc", "abcdef", 6) < 0, __FILE__, __LINE__);
}

static void test_strncmp_empty_strings(void)
{
    expect_eq_int("ft_strncmp both empty", ft_strncmp("", "", 5), 0, __FILE__, __LINE__);
    expect_true("ft_strncmp empty vs nonempty", ft_strncmp("", "a", 1) < 0, __FILE__, __LINE__);
}

static void test_strncmp_sign_only_vs_ref(void)
{
    struct pair { const char *a; const char *b; };
    struct pair pairs[] = {
        {"hello", "hello"}, {"hello", "world"}, {"abc", "abd"}, {"", "a"},
        {"\xFF", "\x7F"}, {"\x80", "\x00"}, {"a\0b", "a\0c"}, {"\0", "\0"}
    };
    for (size_t i = 0; i < sizeof(pairs)/sizeof(pairs[0]); i++) {
        for (size_t n = 0; n <= 5; n++) {
            int got = ft_strncmp(pairs[i].a, pairs[i].b, n);
            int want = ref_strncmp_sign(pairs[i].a, pairs[i].b, n);
            int got_sign = (got > 0) - (got < 0);
            int want_sign = (want > 0) - (want < 0);
            char expr[128];
            snprintf(expr, sizeof(expr), "ft_strncmp sign pair %zu n=%zu", i, n);
            expect_eq_int(expr, got_sign, want_sign, __FILE__, __LINE__);
        }
    }
}

TEST("ft_strncmp n=0", test_strncmp_n_zero);
TEST("ft_strncmp stops at NUL", test_strncmp_stops_at_nul);
TEST("ft_strncmp difference beyond n", test_strncmp_difference_beyond_n);
TEST("ft_strncmp unsigned semantics", test_strncmp_unsigned_semantics);
TEST("ft_strncmp prefix cases", test_strncmp_prefix_cases);
TEST("ft_strncmp empty strings", test_strncmp_empty_strings);
TEST("ft_strncmp sign-only vs ref", test_strncmp_sign_only_vs_ref);

/* ---- ft_strnstr ---- */

static void test_strnstr_empty_needle(void)
{
    expect_eq_int("ft_strnstr empty needle", ft_strnstr("haystack", "", 8) - "haystack", 0, __FILE__, __LINE__);
}

static void test_strnstr_len_zero(void)
{
    expect_true("ft_strnstr len=0 non-empty needle", ft_strnstr("haystack", "needle", 0) == NULL, __FILE__, __LINE__);
    expect_eq_int("ft_strnstr len=0 empty needle", ft_strnstr("haystack", "", 0) - "haystack", 0, __FILE__, __LINE__);
}

static void test_strnstr_needle_longer_than_haystack(void)
{
    expect_true("ft_strnstr needle longer", ft_strnstr("hi", "hello", 2) == NULL, __FILE__, __LINE__);
}

static void test_strnstr_match_straddling_len(void)
{
    char hay[] = "abcdef";
    expect_true("ft_strnstr match at len boundary", ft_strnstr(hay, "def", 6) != NULL, __FILE__, __LINE__);
    expect_true("ft_strnstr match past len", ft_strnstr(hay, "def", 5) == NULL, __FILE__, __LINE__);
}

static void test_strnstr_repeated_prefix(void)
{
    char hay[] = "aaaaab";
    char *res = ft_strnstr(hay, "aab", 6);
    expect_true("ft_strnstr repeated prefix", res != NULL, __FILE__, __LINE__);
    expect_eq_int("ft_strnstr repeated prefix pos", res - hay, 3, __FILE__, __LINE__);
}

static void test_strnstr_no_overread_guard(void)
{
    Gbuf g = gbuf_new(20);
    memcpy(gbuf_at(g, 0), "hello\0world", 12);
    char *res = ft_strnstr(gbuf_at(g, 0), "world", 6);
    expect_true("ft_strnstr no overread past NUL", res == NULL, __FILE__, __LINE__);
    gbuf_free(g);
}

static void test_strnstr_pointer_identity(void)
{
    char hay[] = "needle in haystack";
    char *res = ft_strnstr(hay, "needle", 18);
    expect_true("ft_strnstr returns haystack start", res == hay, __FILE__, __LINE__);
}

static void test_strnstr_vs_ref(void)
{
    const char *haystacks[] = {"hello world", "aaaaab", "abc", "", "a", "needle"};
    const char *needles[] = {"world", "aab", "abc", "", "a", "needle", "x"};
    for (size_t hi = 0; hi < sizeof(haystacks)/sizeof(haystacks[0]); hi++) {
        for (size_t ni = 0; ni < sizeof(needles)/sizeof(needles[0]); ni++) {
            for (size_t len = 0; len <= strlen(haystacks[hi]) + 1; len++) {
                char *got = ft_strnstr(haystacks[hi], needles[ni], len);
                char *want = ref_strnstr(haystacks[hi], needles[ni], len);
                char expr[128];
                snprintf(expr, sizeof(expr), "ft_strnstr vs ref hay=%zu need=%zu len=%zu", hi, ni, len);
                if (want == NULL)
                    expect_true(expr, got == NULL, __FILE__, __LINE__);
                else
                    expect_eq_int(expr, got - haystacks[hi], want - haystacks[hi], __FILE__, __LINE__);
            }
        }
    }
}

TEST("ft_strnstr empty needle", test_strnstr_empty_needle);
TEST("ft_strnstr len=0", test_strnstr_len_zero);
TEST("ft_strnstr needle longer than haystack", test_strnstr_needle_longer_than_haystack);
TEST("ft_strnstr match straddling len", test_strnstr_match_straddling_len);
TEST("ft_strnstr repeated prefix", test_strnstr_repeated_prefix);
TEST("ft_strnstr no overread past NUL", test_strnstr_no_overread_guard);
TEST("ft_strnstr pointer identity", test_strnstr_pointer_identity);
TEST("ft_strnstr vs ref", test_strnstr_vs_ref);

/* ---- ft_strlcpy ---- */

static void test_strlcpy_returns_total_length(void)
{
    char dst[20];
    expect_eq_int("ft_strlcpy returns src len", ft_strlcpy(dst, "hello", 20), 5, __FILE__, __LINE__);
    expect_eq_int("ft_strlcpy returns src len trunc", ft_strlcpy(dst, "hello", 3), 5, __FILE__, __LINE__);
}

static void test_strlcpy_size_zero(void)
{
    unsigned char dst[10] = {0xAA};
    expect_eq_int("ft_strlcpy size=0", ft_strlcpy((char *)dst, "hello", 0), 5, __FILE__, __LINE__);
    expect_true("ft_strlcpy size=0 dst untouched", dst[0] == 0xAA, __FILE__, __LINE__);
}

static void test_strlcpy_size_one(void)
{
    unsigned char dst[10] = {0xAA};
    expect_eq_int("ft_strlcpy size=1", ft_strlcpy((char *)dst, "hello", 1), 5, __FILE__, __LINE__);
    expect_true("ft_strlcpy size=1 NUL only", dst[0] == '\0', __FILE__, __LINE__);
}

static void test_strlcpy_size_exact_fit(void)
{
    char dst[6];
    expect_eq_int("ft_strlcpy size=len+1", ft_strlcpy(dst, "hello", 6), 5, __FILE__, __LINE__);
    expect_true("ft_strlcpy exact fit content", strcmp(dst, "hello") == 0, __FILE__, __LINE__);
}

static void test_strlcpy_size_truncated(void)
{
    char dst[4];
    expect_eq_int("ft_strlcpy size=4", ft_strlcpy(dst, "hello", 4), 5, __FILE__, __LINE__);
    expect_true("ft_strlcpy truncated content", strcmp(dst, "hel") == 0, __FILE__, __LINE__);
    expect_true("ft_strlcpy truncated NUL", dst[3] == '\0', __FILE__, __LINE__);
}

static void test_strlcpy_terminates_when_size_gt_zero(void)
{
    for (size_t sz = 1; sz <= 10; sz++) {
        char dst[20] = {0xFF};
        ft_strlcpy(dst, "abc", sz);
        expect_true("ft_strlcpy terminates sz>0", dst[sz < 4 ? sz - 1 : 3] == '\0', __FILE__, __LINE__);
    }
}

static void test_strlcpy_no_nul_padding_canaries(void)
{
    Gbuf g = gbuf_new(32);
    unsigned char *dst = (unsigned char *)gbuf_at(g, 0);
    memset(dst, 0xAA, 32);
    ft_strlcpy((char *)dst, "hi", 10);
    expect_true("ft_strlcpy no pad after NUL", dst[3] == 0xAA, __FILE__, __LINE__);
    expect_true("ft_strlcpy canary after", dst[4] == 0xAA, __FILE__, __LINE__);
    gbuf_free(g);
}

static void test_strlcpy_vs_ref(void)
{
    const char *srcs[] = {"", "a", "hello", "longer string here", "\xFF\xFE\x00"};
    size_t sizes[] = {0, 1, 2, 5, 10, 20};
    for (size_t si = 0; si < sizeof(srcs)/sizeof(srcs[0]); si++) {
        for (size_t sz_i = 0; sz_i < sizeof(sizes)/sizeof(sizes[0]); sz_i++) {
            size_t sz = sizes[sz_i];
            char dst1[64] = {0}, dst2[64] = {0};
            size_t got = ft_strlcpy(dst1, srcs[si], sz);
            size_t want = ref_strlcpy(dst2, srcs[si], sz);
            char expr[128];
            snprintf(expr, sizeof(expr), "ft_strlcpy vs ref src=%zu sz=%zu", si, sz);
            expect_eq_int(expr, got, want, __FILE__, __LINE__);
            expect_mem_eq(expr, dst1, dst2, sz < 64 ? sz : 64, __FILE__, __LINE__);
        }
    }
}

TEST("ft_strlcpy returns total length", test_strlcpy_returns_total_length);
TEST("ft_strlcpy size=0", test_strlcpy_size_zero);
TEST("ft_strlcpy size=1", test_strlcpy_size_one);
TEST("ft_strlcpy size=exact fit", test_strlcpy_size_exact_fit);
TEST("ft_strlcpy size=truncated", test_strlcpy_size_truncated);
TEST("ft_strlcpy terminates when size>0", test_strlcpy_terminates_when_size_gt_zero);
TEST("ft_strlcpy no NUL padding (canaries)", test_strlcpy_no_nul_padding_canaries);
TEST("ft_strlcpy vs ref", test_strlcpy_vs_ref);

/* ---- ft_strlcat ---- */

static void test_strlcat_returns_total_length(void)
{
    char dst[20] = "hi";
    expect_eq_int("ft_strlcat returns dst+src", ft_strlcat(dst, "there", 20), 2 + 5, __FILE__, __LINE__);
    expect_true("ft_strlcat content", strcmp(dst, "hithere") == 0, __FILE__, __LINE__);
}

static void test_strlcat_size_zero(void)
{
    char dst[10] = "abc";
    expect_eq_int("ft_strlcat size=0", ft_strlcat(dst, "def", 0), 3, __FILE__, __LINE__);
}

static void test_strlcat_size_one(void)
{
    char dst[10] = "abc";
    expect_eq_int("ft_strlcat size=1", ft_strlcat(dst, "def", 1), 1 + 3, __FILE__, __LINE__);
}

static void test_strlcat_size_lt_dst_len(void)
{
    char dst[10] = "hello";
    expect_eq_int("ft_strlcat size<dstlen", ft_strlcat(dst, "world", 3), 3 + 5, __FILE__, __LINE__);
}

static void test_strlcat_size_exact_dst_len(void)
{
    char dst[10] = "hello";
    expect_eq_int("ft_strlcat size=dstlen", ft_strlcat(dst, "world", 5), 5 + 5, __FILE__, __LINE__);
}

static void test_strlcat_terminates_when_size_gt_dstlen(void)
{
    char dst[20] = "hi";
    ft_strlcat(dst, "there", 8);
    expect_true("ft_strlcat NUL terminated", dst[7] == '\0', __FILE__, __LINE__);
}

static void test_strlcat_no_nul_padding_canaries(void)
{
    Gbuf g = gbuf_new(32);
    unsigned char *dst = (unsigned char *)gbuf_at(g, 0);
    strcpy((char *)dst, "hi");
    for (size_t i = 3; i < 32; i++) dst[i] = 0xAA;
    ft_strlcat((char *)dst, "there", 10);
    expect_true("ft_strlcat no pad after NUL", dst[8] == 0xAA, __FILE__, __LINE__);
    gbuf_free(g);
}

static void test_strlcat_dst_unterminated_within_size_guard(void)
{
    Gbuf g = gbuf_new(20);
    unsigned char *dst = (unsigned char *)gbuf_at(g, 0);
    dst[0] = 'a'; dst[1] = 'b'; dst[2] = 'c'; dst[3] = 0xAA; /* no NUL in first 4 */
    size_t res = ft_strlcat((char *)dst, "def", 3);
    expect_eq_int("ft_strlcat unterminated size=3", res, 3 + 3, __FILE__, __LINE__);
    expect_true("ft_strlcat unterminated canary intact", dst[3] == 0xAA, __FILE__, __LINE__);
    gbuf_free(g);
}

static void test_strlcat_vs_ref(void)
{
    const char *dsts[] = {"", "a", "hello", "hi"};
    const char *srcs[] = {"", "x", "world", "there"};
    size_t sizes[] = {0, 1, 2, 5, 10, 20};
    for (size_t di = 0; di < sizeof(dsts)/sizeof(dsts[0]); di++) {
        for (size_t si = 0; si < sizeof(srcs)/sizeof(srcs[0]); si++) {
            for (size_t sz_i = 0; sz_i < sizeof(sizes)/sizeof(sizes[0]); sz_i++) {
                size_t sz = sizes[sz_i];
                char dst1[64], dst2[64];
                strcpy(dst1, dsts[di]);
                strcpy(dst2, dsts[di]);
                size_t got = ft_strlcat(dst1, srcs[si], sz);
                size_t want = ref_strlcat(dst2, srcs[si], sz);
                char expr[128];
                snprintf(expr, sizeof(expr), "ft_strlcat vs ref dst=%zu src=%zu sz=%zu", di, si, sz);
                expect_eq_int(expr, got, want, __FILE__, __LINE__);
                size_t cmplen = sz < 64 ? sz : 64;
                expect_mem_eq(expr, dst1, dst2, cmplen, __FILE__, __LINE__);
            }
        }
    }
}

TEST("ft_strlcat returns total length", test_strlcat_returns_total_length);
TEST("ft_strlcat size=0", test_strlcat_size_zero);
TEST("ft_strlcat size=1", test_strlcat_size_one);
TEST("ft_strlcat size<dstlen", test_strlcat_size_lt_dst_len);
TEST("ft_strlcat size=dstlen", test_strlcat_size_exact_dst_len);
TEST("ft_strlcat terminates when size>dstlen", test_strlcat_terminates_when_size_gt_dstlen);
TEST("ft_strlcat no NUL padding (canaries)", test_strlcat_no_nul_padding_canaries);
TEST("ft_strlcat dst unterminated within size guard", test_strlcat_dst_unterminated_within_size_guard);
TEST("ft_strlcat vs ref", test_strlcat_vs_ref);