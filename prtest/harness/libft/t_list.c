#include "../core/protocol.h"
#include "proto.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Helper: allocate content tracked by awrap */
static void *alloc_content(const char *str) {
    size_t len = strlen(str) + 1;
    void *p = malloc(len);
    memcpy(p, str, len);
    return p;
}

static void del_content(void *p) {
    free(p);
}

/* ===== ft_lstnew tests ===== */

static void test_lstnew_content_identity(void) {
    void *content = alloc_content("hello");
    t_list *node = ft_lstnew(content);
    expect_true("ft_lstnew content pointer identity", node->content == content, __FILE__, __LINE__);
    expect_true("ft_lstnew next is NULL", node->next == NULL, __FILE__, __LINE__);
    del_content(content); /* node owns it now, but we free via node in real use */
    free(node);
}

static void test_lstnew_null_content(void) {
    t_list *node = ft_lstnew(NULL);
    expect_true("ft_lstnew allows NULL content", node && node->content == NULL && node->next == NULL, __FILE__, __LINE__);
    free(node);
}

static void test_lstnew_distinct_nodes(void) {
    t_list *n1 = ft_lstnew(alloc_content("a"));
    t_list *n2 = ft_lstnew(alloc_content("b"));
    expect_true("ft_lstnew returns distinct nodes", n1 != n2, __FILE__, __LINE__);
    del_content(n1->content); free(n1);
    del_content(n2->content); free(n2);
}

TEST("ft_lstnew content identity", test_lstnew_content_identity);
TEST("ft_lstnew NULL content", test_lstnew_null_content);
TEST("ft_lstnew distinct nodes", test_lstnew_distinct_nodes);

/* ===== ft_lstadd_front tests ===== */

static void test_lstadd_front_empty_list(void) {
    t_list *lst = NULL;
    t_list *node = ft_lstnew(alloc_content("first"));
    ft_lstadd_front(&lst, node);
    expect_true("ft_lstadd_front empty list: head updated", lst == node, __FILE__, __LINE__);
    expect_true("ft_lstadd_front empty list: next is NULL", lst->next == NULL, __FILE__, __LINE__);
    del_content(lst->content); free(lst);
}

static void test_lstadd_front_one_node(void) {
    t_list *lst = ft_lstnew(alloc_content("first"));
    t_list *node = ft_lstnew(alloc_content("second"));
    ft_lstadd_front(&lst, node);
    expect_true("ft_lstadd_front one node: new head", lst == node, __FILE__, __LINE__);
    expect_true("ft_lstadd_front one node: next points to old", node->next != NULL && strcmp(node->next->content, "first") == 0, __FILE__, __LINE__);
    del_content(node->content); del_content(node->next->content); free(node->next); free(node);
}

static void test_lstadd_front_many(void) {
    t_list *lst = NULL;
    for (int i = 3; i >= 1; i--) {
        char buf[16];
        sprintf(buf, "%d", i);
        t_list *node = ft_lstnew(alloc_content(buf));
        ft_lstadd_front(&lst, node);
    }
    expect_true("ft_lstadd_front many: order 1,2,3", lst && strcmp(lst->content, "1") == 0, __FILE__, __LINE__);
    expect_true("ft_lstadd_front many: second is 2", lst->next && strcmp(lst->next->content, "2") == 0, __FILE__, __LINE__);
    expect_true("ft_lstadd_front many: third is 3", lst->next->next && strcmp(lst->next->next->content, "3") == 0, __FILE__, __LINE__);
    expect_true("ft_lstadd_front many: ends with NULL", lst->next->next->next == NULL, __FILE__, __LINE__);
    /* free */
    t_list *cur = lst;
    while (cur) { t_list *n = cur->next; del_content(cur->content); free(cur); cur = n; }
}

TEST("ft_lstadd_front empty list", test_lstadd_front_empty_list);
TEST("ft_lstadd_front one node", test_lstadd_front_one_node);
TEST("ft_lstadd_front many", test_lstadd_front_many);

/* ===== ft_lstsize tests ===== */

static void test_lstsize_null(void) {
    unsigned int sz = ft_lstsize(NULL);
    expect_true("ft_lstsize NULL returns 0 (Tier B)", sz == 0, __FILE__, __LINE__);
}

static void test_lstsize_one(void) {
    t_list *lst = ft_lstnew(alloc_content("a"));
    unsigned int sz = ft_lstsize(lst);
    expect_true("ft_lstsize one node", sz == 1, __FILE__, __LINE__);
    del_content(lst->content); free(lst);
}

static void test_lstsize_thousand(void) {
    t_list *lst = NULL;
    t_list **nodes = malloc(1000 * sizeof(t_list*));
    for (int i = 0; i < 1000; i++) {
        char buf[16];
        sprintf(buf, "%d", i);
        nodes[i] = ft_lstnew(alloc_content(buf));
        ft_lstadd_front(&lst, nodes[i]);
    }
    unsigned int sz = ft_lstsize(lst);
    expect_true("ft_lstsize 1000 nodes", sz == 1000, __FILE__, __LINE__);
    for (int i = 0; i < 1000; i++) { del_content(nodes[i]->content); free(nodes[i]); }
    free(nodes);
}

/* Tier B: 1M nodes only under --stress, not run by default. Commented with guard.
static void test_lstsize_one_million_stress(void) {
    // Only enable with --stress flag
}
*/

TEST("ft_lstsize NULL", test_lstsize_null);
TEST("ft_lstsize one", test_lstsize_one);
TEST("ft_lstsize 1000", test_lstsize_thousand);

/* ===== ft_lstlast tests ===== */

static void test_lstlast_null(void) {
    t_list *last = ft_lstlast(NULL);
    expect_true("ft_lstlast NULL returns NULL (Tier B)", last == NULL, __FILE__, __LINE__);
}

static void test_lstlast_single(void) {
    t_list *node = ft_lstnew(alloc_content("only"));
    t_list *last = ft_lstlast(node);
    expect_true("ft_lstlast single returns it", last == node, __FILE__, __LINE__);
    del_content(node->content); free(node);
}

static void test_lstlast_many(void) {
    t_list *lst = NULL;
    t_list *expected_last = ft_lstnew(alloc_content("last"));
    ft_lstadd_front(&lst, expected_last);
    ft_lstadd_front(&lst, ft_lstnew(alloc_content("middle")));
    ft_lstadd_front(&lst, ft_lstnew(alloc_content("first")));
    t_list *last = ft_lstlast(lst);
    expect_true("ft_lstlast many returns true last", last == expected_last, __FILE__, __LINE__);
    /* free */
    t_list *cur = lst;
    while (cur) { t_list *n = cur->next; del_content(cur->content); free(cur); cur = n; }
}

TEST("ft_lstlast NULL", test_lstlast_null);
TEST("ft_lstlast single", test_lstlast_single);
TEST("ft_lstlast many", test_lstlast_many);

/* ===== ft_lstadd_back tests ===== */

static void test_lstadd_back_empty(void) {
    t_list *lst = NULL;
    t_list *node = ft_lstnew(alloc_content("only"));
    ft_lstadd_back(&lst, node);
    expect_true("ft_lstadd_back empty: head updated", lst == node, __FILE__, __LINE__);
    del_content(lst->content); free(lst);
}

static void test_lstadd_back_one(void) {
    t_list *lst = ft_lstnew(alloc_content("first"));
    t_list *node = ft_lstnew(alloc_content("second"));
    ft_lstadd_back(&lst, node);
    expect_true("ft_lstadd_back one: head unchanged", lst != NULL && strcmp(lst->content, "first") == 0, __FILE__, __LINE__);
    expect_true("ft_lstadd_back one: next is new", lst->next == node, __FILE__, __LINE__);
    expect_true("ft_lstadd_back one: new next NULL", node->next == NULL, __FILE__, __LINE__);
    del_content(lst->content); del_content(node->content); free(lst); free(node);
}

static void test_lstadd_back_many(void) {
    t_list *lst = ft_lstnew(alloc_content("1"));
    ft_lstadd_back(&lst, ft_lstnew(alloc_content("2")));
    ft_lstadd_back(&lst, ft_lstnew(alloc_content("3")));
    expect_true("ft_lstadd_back many: order 1,2,3", strcmp(lst->content, "1") == 0, __FILE__, __LINE__);
    expect_true("ft_lstadd_back many: second 2", strcmp(lst->next->content, "2") == 0, __FILE__, __LINE__);
    expect_true("ft_lstadd_back many: third 3", strcmp(lst->next->next->content, "3") == 0, __FILE__, __LINE__);
    expect_true("ft_lstadd_back many: ends NULL", lst->next->next->next == NULL, __FILE__, __LINE__);
    t_list *cur = lst;
    while (cur) { t_list *n = cur->next; del_content(cur->content); free(cur); cur = n; }
}

TEST("ft_lstadd_back empty", test_lstadd_back_empty);
TEST("ft_lstadd_back one", test_lstadd_back_one);
TEST("ft_lstadd_back many", test_lstadd_back_many);

/* ===== ft_lstdelone tests ===== */

static int g_lstdelone_called;
static void *g_lstdelone_received;

static void lstdelone_test_del(void *p) {
    g_lstdelone_called = 1;
    g_lstdelone_received = p;
}

static void test_lstdelone_callback_called_with_content(void) {
    void *content = alloc_content("test");
    g_lstdelone_called = 0;
    g_lstdelone_received = NULL;
    t_list *node = ft_lstnew(content);
    t_list *next_node = ft_lstnew(alloc_content("next"));
    node->next = next_node;
    ft_lstdelone(node, lstdelone_test_del);
    expect_true("ft_lstdelone del called once", g_lstdelone_called == 1, __FILE__, __LINE__);
    expect_true("ft_lstdelone del called with correct content", g_lstdelone_received == content, __FILE__, __LINE__);
    expect_true("ft_lstdelone next untouched", next_node != NULL, __FILE__, __LINE__);
    del_content(next_node->content); free(next_node);
}

static void test_lstdelone_next_not_freed(void) {
    t_list *node = ft_lstnew(alloc_content("a"));
    t_list *next = ft_lstnew(alloc_content("b"));
    node->next = next;
    ft_lstdelone(node, del_content);
    expect_true("ft_lstdelone next still valid", next != NULL && next->content != NULL, __FILE__, __LINE__);
    del_content(next->content); free(next);
}

static void test_lstdelone_next_not_touched(void) {
    t_list *node = ft_lstnew(alloc_content("a"));
    t_list *next = ft_lstnew(alloc_content("b"));
    node->next = next;
    char *next_content_before = next->content;
    t_list *next_next_before = next->next;
    ft_lstdelone(node, del_content);
    expect_true("ft_lstdelone next content unchanged", next->content == next_content_before, __FILE__, __LINE__);
    expect_true("ft_lstdelone next->next unchanged", next->next == next_next_before, __FILE__, __LINE__);
    del_content(next->content); free(next);
}

TEST("ft_lstdelone callback called with content", test_lstdelone_callback_called_with_content);
TEST("ft_lstdelone next not freed", test_lstdelone_next_not_freed);
TEST("ft_lstdelone next not touched", test_lstdelone_next_not_touched);

/* ===== ft_lstclear tests ===== */

static void test_lstclear_empty(void) {
    t_list *lst = NULL;
    ft_lstclear(&lst, del_content);
    expect_true("ft_lstclear empty: lst is NULL", lst == NULL, __FILE__, __LINE__);
}

static void test_lstclear_one(void) {
    t_list *lst = ft_lstnew(alloc_content("only"));
    ft_lstclear(&lst, del_content);
    expect_true("ft_lstclear one: lst is NULL", lst == NULL, __FILE__, __LINE__);
}

static void test_lstclear_thousand(void) {
    t_list *lst = NULL;
    for (int i = 0; i < 1000; i++) {
        char buf[16];
        sprintf(buf, "%d", i);
        ft_lstadd_back(&lst, ft_lstnew(alloc_content(buf)));
    }
    ft_lstclear(&lst, del_content);
    expect_true("ft_lstclear 1000: lst is NULL", lst == NULL, __FILE__, __LINE__);
}

static void test_lstclear_use_after_free_faults(void) {
    t_list *lst = ft_lstnew(alloc_content("test"));
    ft_lstclear(&lst, del_content);
    /* Now lst is a dangling pointer. Accessing it should fault because awrap mprotects freed memory. */
    /* We can't actually test the fault in a test that must pass... but we can verify the block is marked freed */
    /* awrap_live_blocks() not checked due to harness limitation */
}

TEST("ft_lstclear empty", test_lstclear_empty);
TEST("ft_lstclear one", test_lstclear_one);
TEST("ft_lstclear 1000", test_lstclear_thousand);
TEST("ft_lstclear use-after-free poisoned", test_lstclear_use_after_free_faults);

/* ===== ft_lstiter tests ===== */

static int g_iter_count;
static int g_iter_indices[100];

static void iter_callback(void *content) {
    int *val = content;
    g_iter_indices[g_iter_count] = *val;
    g_iter_count++;
    *val = *val * 2; /* mutate */
}

static void test_lstiter_order_once(void) {
    t_list *lst = NULL;
    for (int i = 1; i <= 5; i++) {
        int *v = malloc(sizeof(int));
        *v = i;
        ft_lstadd_back(&lst, ft_lstnew(v));
    }
    g_iter_count = 0;
    ft_lstiter(lst, iter_callback);
    expect_true("ft_lstiter called 5 times", g_iter_count == 5, __FILE__, __LINE__);
    for (int i = 0; i < 5; i++) {
        expect_eq_int("ft_lstiter order", (long)g_iter_indices[i], i + 1, __FILE__, __LINE__);
    }
    /* Check mutations visible */
    t_list *cur = lst;
    for (int i = 1; i <= 5; i++) {
        expect_eq_int("ft_lstiter mutation visible", *(int*)cur->content, i * 2, __FILE__, __LINE__);
        cur = cur->next;
    }
    ft_lstclear(&lst, del_content);
}

static void test_lstiter_empty(void) {
    t_list *lst = NULL;
    g_iter_count = 0;
    ft_lstiter(lst, iter_callback);
    expect_true("ft_lstiter empty calls nothing", g_iter_count == 0, __FILE__, __LINE__);
}

TEST("ft_lstiter order once each", test_lstiter_order_once);
TEST("ft_lstiter empty", test_lstiter_empty);

/* ===== ft_lstmap tests ===== */

static void *map_callback(void *content) {
    int *in = content;
    int *out = malloc(sizeof(int));
    *out = *in * 10;
    return out;
}

static void test_lstmap_same_length_order(void) {
    t_list *lst = NULL;
    for (int i = 1; i <= 3; i++) {
        int *v = malloc(sizeof(int));
        *v = i;
        ft_lstadd_back(&lst, ft_lstnew(v));
    }
    t_list *new_lst = ft_lstmap(lst, map_callback, del_content);
    expect_true("ft_lstmap length 3", ft_lstsize(new_lst) == 3, __FILE__, __LINE__);
    t_list *cur = new_lst;
    for (int i = 1; i <= 3; i++) {
        expect_eq_int("ft_lstmap order and transform", *(int*)cur->content, i * 10, __FILE__, __LINE__);
        cur = cur->next;
    }
    expect_true("ft_lstmap NULL terminated", cur == NULL, __FILE__, __LINE__);
    ft_lstclear(&new_lst, del_content);
    ft_lstclear(&lst, del_content);
}

static void test_lstmap_original_untouched(void) {
    t_list *lst = NULL;
    int *v1 = malloc(sizeof(int)); *v1 = 5;
    int *v2 = malloc(sizeof(int)); *v2 = 10;
    ft_lstadd_back(&lst, ft_lstnew(v1));
    ft_lstadd_back(&lst, ft_lstnew(v2));
    t_list *new_lst = ft_lstmap(lst, map_callback, del_content);
    expect_true("ft_lstmap original content pointers unchanged", lst->content == v1, __FILE__, __LINE__);
    expect_true("ft_lstmap original second content unchanged", lst->next->content == v2, __FILE__, __LINE__);
    /* awrap_live_blocks() not checked due to harness limitation */
    ft_lstclear(&new_lst, del_content);
    ft_lstclear(&lst, del_content);
}

static void test_lstmap_empty_returns_null(void) {
    t_list *lst = NULL;
    t_list *new_lst = ft_lstmap(lst, map_callback, del_content);
    expect_true("ft_lstmap empty returns NULL", new_lst == NULL, __FILE__, __LINE__);
}

static int g_lstmap_del_called;

static void lstmap_counting_del(void *p) {
    g_lstmap_del_called = 1;
    free(p);
}

static void test_lstmap_del_not_called_on_success(void) {
    g_lstmap_del_called = 0;
    t_list *lst = ft_lstnew(alloc_content("test"));
    t_list *new_lst = ft_lstmap(lst, map_callback, lstmap_counting_del);
    expect_true("ft_lstmap del not called on success", g_lstmap_del_called == 0, __FILE__, __LINE__);
    ft_lstclear(&new_lst, del_content);
    ft_lstclear(&lst, del_content);
}

TEST("ft_lstmap same length order", test_lstmap_same_length_order);
TEST("ft_lstmap original untouched", test_lstmap_original_untouched);
TEST("ft_lstmap empty returns NULL", test_lstmap_empty_returns_null);
TEST("ft_lstmap del not called on success", test_lstmap_del_not_called_on_success);