#include "../core/protocol.h"
#include "proto.h"
#include <limits.h>
#include <stdio.h>

#define EXPECT_TRUE(expr, cond)  expect_true(#expr, (cond), __FILE__, __LINE__)
#define EXPECT_EQ(expr, got, want)  expect_eq_int(#expr, (got), (want), __FILE__, __LINE__)

static void test_isalpha_exhaustive(void) {
    for (int c = 0; c <= 255; c++) {
        int std = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        int ft = ft_isalpha(c);
        EXPECT_EQ("isalpha", ft, std ? 1 : 0);
    }
    EXPECT_EQ("isalpha EOF", ft_isalpha(EOF), 0);
}

static void test_isdigit_exhaustive(void) {
    for (int c = 0; c <= 255; c++) {
        int std = (c >= '0' && c <= '9');
        int ft = ft_isdigit(c);
        EXPECT_EQ("isdigit", ft, std ? 1 : 0);
    }
    EXPECT_EQ("isdigit EOF", ft_isdigit(EOF), 0);
}

static void test_isalnum_exhaustive(void) {
    for (int c = 0; c <= 255; c++) {
        int std = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
        int ft = ft_isalnum(c);
        EXPECT_EQ("isalnum", ft, std ? 1 : 0);
    }
    EXPECT_EQ("isalnum EOF", ft_isalnum(EOF), 0);
}

static void test_isascii_exhaustive(void) {
    for (int c = 0; c <= 255; c++) {
        int std = (c >= 0 && c <= 127);
        int ft = ft_isascii(c);
        EXPECT_EQ("isascii", ft, std ? 1 : 0);
    }
    EXPECT_EQ("isascii 256", ft_isascii(256), 0);
    EXPECT_EQ("isascii 1000", ft_isascii(1000), 0);
    EXPECT_EQ("isascii -1", ft_isascii(-1), 0);
    EXPECT_EQ("isascii EOF", ft_isascii(EOF), 0);
}

static void test_isprint_exhaustive(void) {
    for (int c = 0; c <= 255; c++) {
        int std = (c >= 32 && c <= 126);
        int ft = ft_isprint(c);
        EXPECT_EQ("isprint", ft, std ? 1 : 0);
    }
    EXPECT_EQ("isprint EOF", ft_isprint(EOF), 0);
}

static void test_toupper_exhaustive(void) {
    for (int c = 0; c <= 255; c++) {
        int std = (c >= 'a' && c <= 'z') ? c - 32 : c;
        int ft = ft_toupper(c);
        EXPECT_EQ("toupper", ft, std);
    }
    EXPECT_EQ("toupper EOF", ft_toupper(EOF), EOF);
    for (int c = 128; c <= 255; c++) {
        int ft = ft_toupper(c);
        EXPECT_EQ("toupper >=128 unchanged", ft, c);
    }
}

static void test_tolower_exhaustive(void) {
    for (int c = 0; c <= 255; c++) {
        int std = (c >= 'A' && c <= 'Z') ? c + 32 : c;
        int ft = ft_tolower(c);
        EXPECT_EQ("tolower", ft, std);
    }
    EXPECT_EQ("tolower EOF", ft_tolower(EOF), EOF);
    for (int c = 128; c <= 255; c++) {
        int ft = ft_tolower(c);
        EXPECT_EQ("tolower >=128 unchanged", ft, c);
    }
}

static void test_strlen_basic(void) {
    EXPECT_EQ("strlen empty", ft_strlen(""), 0);
    EXPECT_EQ("strlen a", ft_strlen("a"), 1);
    EXPECT_EQ("strlen hello", ft_strlen("hello"), 5);
}

static void test_strlen_alignment_edges(void) {
    char buf[512];
    for (size_t i = 0; i < sizeof(buf); i++) buf[i] = 'x';
    for (size_t len = 0; len <= 300; len++) {
        buf[len] = '\0';
        EXPECT_EQ("strlen alignment", ft_strlen(buf), len);
        buf[len] = 'x';
    }
    for (size_t len = 4090; len <= 4096; len++) {
        if (len < sizeof(buf)) {
            buf[len] = '\0';
            EXPECT_EQ("strlen page edge", ft_strlen(buf), len);
            buf[len] = 'x';
        }
    }
}

static void test_strlen_guard_page(void) {
    Gbuf g = gbuf_new(100);
    if (!g.base) return;
    for (size_t i = 0; i < 100; i++) g.base[i] = 'a';
    g.base[50] = '\0';
    EXPECT_EQ("strlen guard page", ft_strlen(g.base), 50);
    gbuf_free(g);
}

TEST("ft_isalpha exhaustive", test_isalpha_exhaustive);
TEST("ft_isdigit exhaustive", test_isdigit_exhaustive);
TEST("ft_isalnum exhaustive", test_isalnum_exhaustive);
TEST("ft_isascii exhaustive", test_isascii_exhaustive);
TEST("ft_isprint exhaustive", test_isprint_exhaustive);
TEST("ft_toupper exhaustive", test_toupper_exhaustive);
TEST("ft_tolower exhaustive", test_tolower_exhaustive);
TEST("ft_strlen basic", test_strlen_basic);
TEST("ft_strlen alignment edges", test_strlen_alignment_edges);
TEST("ft_strlen guard page", test_strlen_guard_page);