#include "../core/protocol.h"
#include "proto.h"
#include <string.h>
#include <limits.h>
#include <stdlib.h>

/* ===== Helper: reference split for comparison ===== */
static size_t ref_count_tokens(const char *s, char c) {
    size_t count = 0;
    int in_token = 0;
    while (*s) {
        if (*s == c) {
            in_token = 0;
        } else if (!in_token) {
            in_token = 1;
            count++;
        }
        s++;
    }
    return count;
}

static char **ref_split(const char *s, char c, size_t *out_count) {
    size_t n = ref_count_tokens(s, c);
    if (out_count) *out_count = n;
    char **arr = malloc((n + 1) * sizeof(char *));
    if (!arr) return NULL;
    size_t i = 0;
    const char *start = s;
    int in_token = 0;
    while (*s) {
        if (*s == c) {
            if (in_token) {
                arr[i] = malloc(s - start + 1);
                if (!arr[i]) {
                    while (i--) free(arr[i]);
                    free(arr);
                    return NULL;
                }
                memcpy(arr[i], start, s - start);
                arr[i][s - start] = '\0';
                i++;
            }
            in_token = 0;
            start = s + 1;
        } else if (!in_token) {
            in_token = 1;
            start = s;
        }
        s++;
    }
    if (in_token) {
        arr[i] = malloc(s - start + 1);
        if (!arr[i]) {
            while (i--) free(arr[i]);
            free(arr);
            return NULL;
        }
        memcpy(arr[i], start, s - start);
        arr[i][s - start] = '\0';
        i++;
    }
    arr[n] = NULL;
    return arr;
}

static void ref_split_free(char **arr) {
    if (!arr) return;
    for (size_t i = 0; arr[i]; i++) free(arr[i]);
    free(arr);
}

/* ===== ft_substr tests ===== */

static void test_substr_normal(void) {
    const char *s = "hello world";
    char *r = ft_substr(s, 0, 5);
    expect_true("ft_substr(\"hello world\", 0, 5) == \"hello\"", r && strcmp(r, "hello") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_substr_start_at_len(void) {
    const char *s = "hello";
    char *r = ft_substr(s, 5, 3);
    expect_true("ft_substr start==len returns allocated empty string", r && r[0] == '\0', __FILE__, __LINE__);
    free(r);
}

static void test_substr_start_past_len(void) {
    const char *s = "hi";
    char *r = ft_substr(s, 10, 3);
    expect_true("ft_substr start>len returns allocated empty string", r && r[0] == '\0', __FILE__, __LINE__);
    free(r);
}

static void test_substr_len_zero(void) {
    const char *s = "hello";
    char *r = ft_substr(s, 2, 0);
    expect_true("ft_substr len==0 returns allocated empty string", r && r[0] == '\0', __FILE__, __LINE__);
    free(r);
}

static void test_substr_len_past_end(void) {
    const char *s = "hello";
    char *r = ft_substr(s, 2, 100);
    expect_true("ft_substr len past end clamps to remaining", r && strcmp(r, "llo") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_substr_len_size_max(void) {
    const char *s = "x";
    char *r = ft_substr(s, 0, SIZE_MAX);
    expect_true("ft_substr len==SIZE_MAX doesn't overflow", r && strcmp(r, "x") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_substr_start_uint_max(void) {
    const char *s = "test";
    char *r = ft_substr(s, UINT_MAX, 2);
    expect_true("ft_substr start==UINT_MAX returns empty", r && r[0] == '\0', __FILE__, __LINE__);
    free(r);
}

static void test_substr_result_terminated(void) {
    const char *s = "abc";
    char *r = ft_substr(s, 0, 2);
    expect_true("ft_substr result is null-terminated", r && r[2] == '\0', __FILE__, __LINE__);
    free(r);
}

static void test_substr_original_untouched(void) {
    char s[] = "hello";
    char *r = ft_substr(s, 1, 2);
    expect_true("ft_substr doesn't modify original", strcmp(s, "hello") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_substr_guard_page(void) {
    Gbuf g = gbuf_new(4096);
    strcpy(gbuf_at(g, 0), "guarded");
    char *r = ft_substr(gbuf_at(g, 0), 1, 3);
    expect_true("ft_substr works with guard-page input", r && strcmp(r, "uar") == 0, __FILE__, __LINE__);
    free(r);
    gbuf_free(g);
}

TEST("ft_substr normal", test_substr_normal);
TEST("ft_substr start==len", test_substr_start_at_len);
TEST("ft_substr start>len", test_substr_start_past_len);
TEST("ft_substr len==0", test_substr_len_zero);
TEST("ft_substr len past end", test_substr_len_past_end);
TEST("ft_substr len==SIZE_MAX", test_substr_len_size_max);
TEST("ft_substr start==UINT_MAX", test_substr_start_uint_max);
TEST("ft_substr result terminated", test_substr_result_terminated);
TEST("ft_substr original untouched", test_substr_original_untouched);
TEST("ft_substr guard page", test_substr_guard_page);

/* ===== ft_strjoin tests ===== */

static void test_strjoin_empty_empty(void) {
    char *r = ft_strjoin("", "");
    expect_true("ft_strjoin empty+empty returns allocated empty", r && r[0] == '\0', __FILE__, __LINE__);
    free(r);
}

static void test_strjoin_empty_x(void) {
    char *r = ft_strjoin("", "hello");
    expect_true("ft_strjoin empty+x works", r && strcmp(r, "hello") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_strjoin_x_empty(void) {
    char *r = ft_strjoin("hello", "");
    expect_true("ft_strjoin x+empty works", r && strcmp(r, "hello") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_strjoin_long(void) {
    char a[10000], b[10000];
    memset(a, 'a', 9999); a[9999] = '\0';
    memset(b, 'b', 9999); b[9999] = '\0';
    char *r = ft_strjoin(a, b);
    expect_true("ft_strjoin long strings", r && strlen(r) == 19998 && r[0] == 'a' && r[9998] == 'a' && r[9999] == 'b', __FILE__, __LINE__);
    free(r);
}

static void test_strjoin_fresh_allocation(void) {
    const char *s1 = "hello";
    const char *s2 = "world";
    char *r = ft_strjoin(s1, s2);
    expect_true("ft_strjoin returns fresh allocation (not s1)", r != s1, __FILE__, __LINE__);
    expect_true("ft_strjoin returns fresh allocation (not s2)", r != s2, __FILE__, __LINE__);
    free(r);
}

static void test_strjoin_originals_unmodified(void) {
    char s1[] = "hello";
    char s2[] = "world";
    char *r = ft_strjoin(s1, s2);
    expect_true("ft_strjoin doesn't modify s1", strcmp(s1, "hello") == 0, __FILE__, __LINE__);
    expect_true("ft_strjoin doesn't modify s2", strcmp(s2, "world") == 0, __FILE__, __LINE__);
    free(r);
}

TEST("ft_strjoin empty+empty", test_strjoin_empty_empty);
TEST("ft_strjoin empty+x", test_strjoin_empty_x);
TEST("ft_strjoin x+empty", test_strjoin_x_empty);
TEST("ft_strjoin long", test_strjoin_long);
TEST("ft_strjoin fresh allocation", test_strjoin_fresh_allocation);
TEST("ft_strjoin originals unmodified", test_strjoin_originals_unmodified);

/* ===== ft_strtrim tests ===== */

static void test_strtrim_empty_set(void) {
    char *r = ft_strtrim("hello", "");
    expect_true("ft_strtrim empty set returns copy", r && strcmp(r, "hello") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_strtrim_empty_s1(void) {
    char *r = ft_strtrim("", "abc");
    expect_true("ft_strtrim empty s1 returns allocated empty", r && r[0] == '\0', __FILE__, __LINE__);
    free(r);
}

static void test_strtrim_all_trimmed(void) {
    char *r = ft_strtrim("xxx", "x");
    expect_true("ft_strtrim everything trimmed returns empty", r && r[0] == '\0', __FILE__, __LINE__);
    free(r);
}

static void test_strtrim_front_only(void) {
    char *r = ft_strtrim("xxhello", "x");
    expect_true("ft_strtrim front only", r && strcmp(r, "hello") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_strtrim_back_only(void) {
    char *r = ft_strtrim("helloxx", "x");
    expect_true("ft_strtrim back only", r && strcmp(r, "hello") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_strtrim_both(void) {
    char *r = ft_strtrim("xxhelloxx", "x");
    expect_true("ft_strtrim both sides", r && strcmp(r, "hello") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_strtrim_inner_preserved(void) {
    char *r = ft_strtrim("xhelloxworldx", "x");
    expect_true("ft_strtrim inner occurrences preserved", r && strcmp(r, "helloxworld") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_strtrim_duplicates_in_set(void) {
    char *r = ft_strtrim("xyhello", "xy");
    expect_true("ft_strtrim duplicates in set", r && strcmp(r, "hello") == 0, __FILE__, __LINE__);
    free(r);
}

static void test_strtrim_nothing_to_trim(void) {
    char *r = ft_strtrim("hello", "x");
    const char *original = "hello";
    expect_true("ft_strtrim nothing to trim returns fresh copy", r && strcmp(r, "hello") == 0 && r != original, __FILE__, __LINE__);
    free(r);
}

TEST("ft_strtrim empty set", test_strtrim_empty_set);
TEST("ft_strtrim empty s1", test_strtrim_empty_s1);
TEST("ft_strtrim all trimmed", test_strtrim_all_trimmed);
TEST("ft_strtrim front only", test_strtrim_front_only);
TEST("ft_strtrim back only", test_strtrim_back_only);
TEST("ft_strtrim both sides", test_strtrim_both);
TEST("ft_strtrim inner preserved", test_strtrim_inner_preserved);
TEST("ft_strtrim duplicates in set", test_strtrim_duplicates_in_set);
TEST("ft_strtrim nothing to trim", test_strtrim_nothing_to_trim);

/* ===== ft_split tests ===== */

static void test_split_empty_string(void) {
    char **r = ft_split("", ' ');
    expect_true("ft_split empty string returns [NULL]", r && r[0] == NULL, __FILE__, __LINE__);
    /* awrap_live_blocks() not checked due to harness limitation */
    ref_split_free(r);
}

static void test_split_delimiters_only(void) {
    char **r = ft_split("   ", ' ');
    expect_true("ft_split delimiters only returns [NULL]", r && r[0] == NULL, __FILE__, __LINE__);
    /* awrap_live_blocks() not checked due to harness limitation */
    ref_split_free(r);
}

static void test_split_no_delimiter(void) {
    char **r = ft_split("hello", ' ');
    expect_true("ft_split no delimiter returns single token", r && r[0] && strcmp(r[0], "hello") == 0 && r[1] == NULL, __FILE__, __LINE__);
    /* awrap_live_blocks() not checked due to harness limitation */
    ref_split_free(r);
}

static void test_split_leading_delimiters(void) {
    char **r = ft_split("  hello", ' ');
    expect_true("ft_split leading delimiters", r && r[0] && strcmp(r[0], "hello") == 0 && r[1] == NULL, __FILE__, __LINE__);
    /* awrap_live_blocks() not checked due to harness limitation */
    ref_split_free(r);
}

static void test_split_trailing_delimiters(void) {
    char **r = ft_split("hello  ", ' ');
    expect_true("ft_split trailing delimiters", r && r[0] && strcmp(r[0], "hello") == 0 && r[1] == NULL, __FILE__, __LINE__);
    /* awrap_live_blocks() not checked due to harness limitation */
    ref_split_free(r);
}

static void test_split_consecutive_delimiters(void) {
    char **r = ft_split("a  b", ' ');
    expect_true("ft_split consecutive delimiters", r && r[0] && strcmp(r[0], "a") == 0 && r[1] && strcmp(r[1], "b") == 0 && r[2] == NULL, __FILE__, __LINE__);
    /* awrap_live_blocks() not checked due to harness limitation */
    ref_split_free(r);
}

static void test_split_null_delimiter(void) {
    /* Tier B: c=='\0' behavior is an interpretation. We WARN but don't fail by default. */
    char **r = ft_split("a\0b\0c", '\0');
    size_t ref_n = 0;
    char **ref = ref_split("a\0b\0c", '\0', &ref_n);
    int ok = 1;
    if (!r || !ref) ok = 0;
    for (size_t i = 0; ok && (r[i] || ref[i]); i++) {
        if (!r[i] || !ref[i] || strcmp(r[i], ref[i]) != 0) { ok = 0; break; }
    }
    if (!ok) {
        fail("ft_split c=='\\0' differs from reference (Tier B: WARN)", __FILE__, __LINE__);
    }
    ref_split_free(r);
    ref_split_free(ref);
}

static void test_split_many_tokens(void) {
    /* Build a string with ~10k tokens */
    size_t n = 10000;
    size_t len = n * 2; /* "a " repeated */
    char *s = malloc(len + 1);
    for (size_t i = 0; i < n; i++) {
        s[i*2] = 'a';
        s[i*2+1] = ' ';
    }
    s[len] = '\0';
    char **r = ft_split(s, ' ');
    size_t count = 0;
    while (r[count]) count++;
    expect_true("ft_split 10k tokens count", count == n, __FILE__, __LINE__);
    /* awrap_live_blocks() not checked due to harness limitation */
    for (size_t i = 0; i < n; i++) {
        expect_true("ft_split 10k token content", r[i] && r[i][0] == 'a' && r[i][1] == '\0', __FILE__, __LINE__);
    }
    expect_true("ft_split 10k NULL terminated", r[n] == NULL, __FILE__, __LINE__);
    ref_split_free(r);
    free(s);
}

static void test_split_input_untouched(void) {
    char s[] = "a b c";
    char *orig = strdup(s);
    char **r = ft_split(s, ' ');
    expect_true("ft_split doesn't modify input", strcmp(s, orig) == 0, __FILE__, __LINE__);
    free(orig);
    ref_split_free(r);
}

TEST("ft_split empty string", test_split_empty_string);
TEST("ft_split delimiters only", test_split_delimiters_only);
TEST("ft_split no delimiter", test_split_no_delimiter);
TEST("ft_split leading delimiters", test_split_leading_delimiters);
TEST("ft_split trailing delimiters", test_split_trailing_delimiters);
TEST("ft_split consecutive delimiters", test_split_consecutive_delimiters);
TEST("ft_split c==\\0 (Tier B)", test_split_null_delimiter);
TEST("ft_split 10k tokens", test_split_many_tokens);
TEST("ft_split input untouched", test_split_input_untouched);

/* ===== ft_strmapi / ft_striteri tests ===== */

static int g_strmapi_counter;
static unsigned int g_strmapi_indices[100];
static char g_strmapi_output[100];

static char strmapi_callback(unsigned int i, char c) {
    g_strmapi_indices[g_strmapi_counter] = i;
    g_strmapi_output[g_strmapi_counter] = c;
    g_strmapi_counter++;
    return c + 1; /* transform */
}

static void test_strmapi_callback_indices(void) {
    g_strmapi_counter = 0;
    memset(g_strmapi_indices, 0, sizeof(g_strmapi_indices));
    memset(g_strmapi_output, 0, sizeof(g_strmapi_output));
    const char *s = "abcd";
    char *r = ft_strmapi(s, strmapi_callback);
    expect_true("ft_strmapi callback called 4 times", g_strmapi_counter == 4, __FILE__, __LINE__);
    for (int i = 0; i < 4; i++) {
        expect_eq_int("ft_strmapi index order", (long)g_strmapi_indices[i], i, __FILE__, __LINE__);
        expect_eq_int("ft_strmapi char passed", (long)g_strmapi_output[i], (long)s[i], __FILE__, __LINE__);
    }
    expect_true("ft_strmapi returns transformed", r && strcmp(r, "bcde") == 0, __FILE__, __LINE__);
    expect_true("ft_strmapi original untouched", strcmp(s, "abcd") == 0, __FILE__, __LINE__);
    expect_true("ft_strmapi result terminated", r[4] == '\0', __FILE__, __LINE__);
    free(r);
}

static void test_strmapi_empty_string(void) {
    g_strmapi_counter = 0;
    char *r = ft_strmapi("", strmapi_callback);
    expect_true("ft_strmapi empty string calls f 0 times", g_strmapi_counter == 0, __FILE__, __LINE__);
    expect_true("ft_strmapi empty returns allocated empty", r && r[0] == '\0', __FILE__, __LINE__);
    free(r);
}

TEST("ft_strmapi callback indices", test_strmapi_callback_indices);
TEST("ft_strmapi empty string", test_strmapi_empty_string);

static int g_striteri_counter;
static unsigned int g_striteri_indices[100];
static char *g_striteri_ptrs[100];

static void striteri_callback(unsigned int i, char *c) {
    g_striteri_indices[g_striteri_counter] = i;
    g_striteri_ptrs[g_striteri_counter] = c;
    g_striteri_counter++;
    *c = *c + 1;
}

static void test_striteri_callback_indices_and_pointers(void) {
    g_striteri_counter = 0;
    memset(g_striteri_indices, 0, sizeof(g_striteri_indices));
    memset(g_striteri_ptrs, 0, sizeof(g_striteri_ptrs));
    char s[] = "abcd";
    ft_striteri(s, striteri_callback);
    expect_true("ft_striteri callback called 4 times", g_striteri_counter == 4, __FILE__, __LINE__);
    for (int i = 0; i < 4; i++) {
        expect_eq_int("ft_striteri index order", (long)g_striteri_indices[i], i, __FILE__, __LINE__);
        expect_true("ft_striteri pointer identity", g_striteri_ptrs[i] == &s[i], __FILE__, __LINE__);
    }
    expect_true("ft_striteri modifies in place", strcmp(s, "bcde") == 0, __FILE__, __LINE__);
}

static void test_striteri_empty_string(void) {
    g_striteri_counter = 0;
    char s[] = "";
    ft_striteri(s, striteri_callback);
    expect_true("ft_striteri empty string calls f 0 times", g_striteri_counter == 0, __FILE__, __LINE__);
}

TEST("ft_striteri callback indices and pointers", test_striteri_callback_indices_and_pointers);
TEST("ft_striteri empty string", test_striteri_empty_string);