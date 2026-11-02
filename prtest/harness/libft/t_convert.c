#include "../core/protocol.h"
#include "proto.h"
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

/* ---- ft_atoi ---- */

static void test_atoi_whitespace_set(void)
{
    const char *ws = " \t\n\v\f\r";
    for (size_t i = 0; i < strlen(ws); i++) {
        char buf[10] = {ws[i], ws[i], '4', '2', '\0'};
        expect_eq_int("ft_atoi whitespace", ft_atoi(buf), 42, __FILE__, __LINE__);
    }
}

static void test_atoi_signs(void)
{
    expect_eq_int("ft_atoi +42", ft_atoi("+42"), 42, __FILE__, __LINE__);
    expect_eq_int("ft_atoi -42", ft_atoi("-42"), -42, __FILE__, __LINE__);
    expect_eq_int("ft_atoi +-1", ft_atoi("+-1"), 0, __FILE__, __LINE__);
    expect_eq_int("ft_atoi --1", ft_atoi("--1"), 0, __FILE__, __LINE__);
    expect_eq_int("ft_atoi '- 1'", ft_atoi("- 1"), 0, __FILE__, __LINE__);
}

static void test_atoi_empty_and_sign_only(void)
{
    expect_eq_int("ft_atoi empty", ft_atoi(""), 0, __FILE__, __LINE__);
    expect_eq_int("ft_atoi spaces", ft_atoi("   "), 0, __FILE__, __LINE__);
    expect_eq_int("ft_atoi sign only +", ft_atoi("+"), 0, __FILE__, __LINE__);
    expect_eq_int("ft_atoi sign only -", ft_atoi("-"), 0, __FILE__, __LINE__);
}

static void test_atoi_leading_zeros(void)
{
    expect_eq_int("ft_atoi 0042", ft_atoi("0042"), 42, __FILE__, __LINE__);
    expect_eq_int("ft_atoi 000", ft_atoi("000"), 0, __FILE__, __LINE__);
}

static void test_atoi_trailing_garbage(void)
{
    expect_eq_int("ft_atoi 42abc", ft_atoi("42abc"), 42, __FILE__, __LINE__);
    expect_eq_int("ft_atoi 123xyz", ft_atoi("123xyz"), 123, __FILE__, __LINE__);
}

static void test_atoi_int_max_min(void)
{
    expect_eq_int("ft_atoi INT_MAX", ft_atoi("2147483647"), INT_MAX, __FILE__, __LINE__);
    expect_eq_int("ft_atoi INT_MIN", ft_atoi("-2147483648"), INT_MIN, __FILE__, __LINE__);
}

static void test_atoi_vs_libc_in_range(void)
{
    const char *tests[] = {
        "0", "1", "-1", "42", "-42", "2147483647", "-2147483648",
        "  123", "\t-456", "+789", "000123", "123abc"
    };
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        int got = ft_atoi(tests[i]);
        int want = atoi(tests[i]);
        char expr[64];
        snprintf(expr, sizeof(expr), "ft_atoi vs libc: %s", tests[i]);
        expect_eq_int(expr, got, want, __FILE__, __LINE__);
    }
}

TEST("ft_atoi whitespace set", test_atoi_whitespace_set);
TEST("ft_atoi signs", test_atoi_signs);
TEST("ft_atoi empty and sign only", test_atoi_empty_and_sign_only);
TEST("ft_atoi leading zeros", test_atoi_leading_zeros);
TEST("ft_atoi trailing garbage", test_atoi_trailing_garbage);
TEST("ft_atoi INT_MAX/INT_MIN", test_atoi_int_max_min);
TEST("ft_atoi vs libc in-range", test_atoi_vs_libc_in_range);

/* ---- ft_itoa ---- */

static void test_itoa_zero(void)
{
    char *s = ft_itoa(0);
    expect_true("ft_itoa 0 non-NULL", s != NULL, __FILE__, __LINE__);
    expect_eq_int("ft_itoa 0 content", strcmp(s, "0"), 0, __FILE__, __LINE__);
    free(s);
}

static void test_itoa_pm_one(void)
{
    char *s1 = ft_itoa(1);
    char *s2 = ft_itoa(-1);
    expect_eq_int("ft_itoa 1", strcmp(s1, "1"), 0, __FILE__, __LINE__);
    expect_eq_int("ft_itoa -1", strcmp(s2, "-1"), 0, __FILE__, __LINE__);
    free(s1); free(s2);
}

static void test_itoa_pm_nine_ten(void)
{
    char *s1 = ft_itoa(9);
    char *s2 = ft_itoa(10);
    char *s3 = ft_itoa(-9);
    char *s4 = ft_itoa(-10);
    expect_eq_int("ft_itoa 9", strcmp(s1, "9"), 0, __FILE__, __LINE__);
    expect_eq_int("ft_itoa 10", strcmp(s2, "10"), 0, __FILE__, __LINE__);
    expect_eq_int("ft_itoa -9", strcmp(s3, "-9"), 0, __FILE__, __LINE__);
    expect_eq_int("ft_itoa -10", strcmp(s4, "-10"), 0, __FILE__, __LINE__);
    free(s1); free(s2); free(s3); free(s4);
}

static void test_itoa_pm_99_100(void)
{
    char *s1 = ft_itoa(99);
    char *s2 = ft_itoa(100);
    char *s3 = ft_itoa(-99);
    char *s4 = ft_itoa(-100);
    expect_eq_int("ft_itoa 99", strcmp(s1, "99"), 0, __FILE__, __LINE__);
    expect_eq_int("ft_itoa 100", strcmp(s2, "100"), 0, __FILE__, __LINE__);
    expect_eq_int("ft_itoa -99", strcmp(s3, "-99"), 0, __FILE__, __LINE__);
    expect_eq_int("ft_itoa -100", strcmp(s4, "-100"), 0, __FILE__, __LINE__);
    free(s1); free(s2); free(s3); free(s4);
}

static void test_itoa_powers_of_ten(void)
{
    int vals[] = {10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
    for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
        char *s1 = ft_itoa(vals[i]);
        char *s2 = ft_itoa(-vals[i]);
        char want1[20], want2[20];
        snprintf(want1, sizeof(want1), "%d", vals[i]);
        snprintf(want2, sizeof(want2), "%d", -vals[i]);
        expect_eq_int("ft_itoa power of 10", strcmp(s1, want1), 0, __FILE__, __LINE__);
        expect_eq_int("ft_itoa neg power of 10", strcmp(s2, want2), 0, __FILE__, __LINE__);
        free(s1); free(s2);
    }
}

static void test_itoa_int_max_min(void)
{
    char *s1 = ft_itoa(INT_MAX);
    char *s2 = ft_itoa(INT_MIN);
    char *s3 = ft_itoa(INT_MIN + 1);
    char want1[20], want2[20], want3[20];
    snprintf(want1, sizeof(want1), "%d", INT_MAX);
    snprintf(want2, sizeof(want2), "%d", INT_MIN);
    snprintf(want3, sizeof(want3), "%d", INT_MIN + 1);
    expect_eq_int("ft_itoa INT_MAX", strcmp(s1, want1), 0, __FILE__, __LINE__);
    expect_eq_int("ft_itoa INT_MIN", strcmp(s2, want2), 0, __FILE__, __LINE__);
    expect_eq_int("ft_itoa INT_MIN+1", strcmp(s3, want3), 0, __FILE__, __LINE__);
    free(s1); free(s2); free(s3);
}

static void test_itoa_no_plus_prefix_no_leading_zeros(void)
{
    char *s1 = ft_itoa(42);
    char *s2 = ft_itoa(010);  /* octal 10 = decimal 8 */
    expect_true("ft_itoa no plus prefix", s1[0] != '+', __FILE__, __LINE__);
    expect_true("ft_itoa no leading zeros", s2[0] != '0' || s2[1] == '\0', __FILE__, __LINE__);
    free(s1); free(s2);
}

static void test_itoa_fresh_allocation(void)
{
    awrap_install();
    char *s = ft_itoa(123);
    expect_true("ft_itoa allocated", s != NULL, __FILE__, __LINE__);
    free(s);
}

static void test_itoa_vs_snprintf(void)
{
    rng_seed(0xDEADBEEF);
    for (int i = 0; i < 10000; i++) {
        int val = (int)(rng_next() % (2LL * INT_MAX)) - INT_MAX;
        char *got = ft_itoa(val);
        char want[20];
        snprintf(want, sizeof(want), "%d", val);
        char expr[64];
        snprintf(expr, sizeof(expr), "ft_itoa vs snprintf %d", val);
        expect_eq_int(expr, strcmp(got, want), 0, __FILE__, __LINE__);
        free(got);
    }
}

/* Tier B: failure injection - deferred due to harness awrap_fail_at limitation */
/* static void test_itoa_failure_injection(void) { ... } */

TEST("ft_itoa zero", test_itoa_zero);
TEST("ft_itoa ±1", test_itoa_pm_one);
TEST("ft_itoa ±9/10", test_itoa_pm_nine_ten);
TEST("ft_itoa ±99/100", test_itoa_pm_99_100);
TEST("ft_itoa powers of ten ±1", test_itoa_powers_of_ten);
TEST("ft_itoa INT_MAX/INT_MIN", test_itoa_int_max_min);
TEST("ft_itoa no plus/leading zeros", test_itoa_no_plus_prefix_no_leading_zeros);
TEST("ft_itoa fresh allocation", test_itoa_fresh_allocation);
TEST("ft_itoa vs snprintf 10k random", test_itoa_vs_snprintf);