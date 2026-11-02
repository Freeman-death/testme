#include "prtest.h"
#include <stddef.h>
#include <string.h>

/* Category enum for the 6 function groups */
typedef enum {
    CAT_CTYPE   = 0,
    CAT_STRING  = 1,
    CAT_MEMORY  = 2,
    CAT_CONVERT = 3,
    CAT_ALLOC   = 4,
    CAT_OUTPUT  = 5,
    CAT_LIST    = 6,
    CAT_COUNT
} SpecCategory;

/* Tier enum: A = spec (must pass), B = convention (warn), C = UB (skip) */
typedef enum {
    TIER_A = 0,
    TIER_B = 1,
    TIER_C = 2
} SpecTier;

/* Max number of allowed external symbols per function */
#define MAX_EXTERNALS 4

/* Single spec entry for one libft function */
typedef struct {
    const char *name;                 /* e.g. "ft_strlen" */
    const char *prototype;            /* full prototype string */
    SpecCategory category;            /* category enum */
    SpecTier tier;                    /* A/B/C tier */
    const char *externals[MAX_EXTERNALS]; /* allowed libc symbols (NULL-terminated) */
} SpecEntry;

/* ---- The 43-function spec table (libft v19.3) ---- */
static const SpecEntry g_spec_table[] = {
    /* ctype (7) - Tier A: spec behavior */
    { "ft_isalpha", "int ft_isalpha(int c)", CAT_CTYPE, TIER_A, { NULL } },
    { "ft_isdigit", "int ft_isdigit(int c)", CAT_CTYPE, TIER_A, { NULL } },
    { "ft_isalnum", "int ft_isalnum(int c)", CAT_CTYPE, TIER_A, { NULL } },
    { "ft_isascii", "int ft_isascii(int c)", CAT_CTYPE, TIER_A, { NULL } },
    { "ft_isprint", "int ft_isprint(int c)", CAT_CTYPE, TIER_A, { NULL } },
    { "ft_toupper", "int ft_toupper(int c)", CAT_CTYPE, TIER_A, { NULL } },
    { "ft_tolower", "int ft_tolower(int c)", CAT_CTYPE, TIER_A, { NULL } },

    /* string (7) - Tier A */
    { "ft_strlen",  "size_t ft_strlen(const char *s)", CAT_STRING, TIER_A, { NULL } },
    { "ft_strlcpy", "size_t ft_strlcpy(char *dst, const char *src, size_t size)", CAT_STRING, TIER_A, { NULL } },
    { "ft_strlcat", "size_t ft_strlcat(char *dst, const char *src, size_t size)", CAT_STRING, TIER_A, { NULL } },
    { "ft_strchr",  "char *ft_strchr(const char *s, int c)", CAT_STRING, TIER_A, { NULL } },
    { "ft_strrchr", "char *ft_strrchr(const char *s, int c)", CAT_STRING, TIER_A, { NULL } },
    { "ft_strnstr", "char *ft_strnstr(const char *hay, const char *need, size_t len)", CAT_STRING, TIER_A, { NULL } },
    { "ft_strncmp", "int ft_strncmp(const char *s1, const char *s2, size_t n)", CAT_STRING, TIER_A, { NULL } },

    /* memory (6) - Tier A */
    { "ft_memset",  "void *ft_memset(void *s, int c, size_t n)", CAT_MEMORY, TIER_A, { NULL } },
    { "ft_bzero",   "void ft_bzero(void *s, size_t n)", CAT_MEMORY, TIER_A, { NULL } },
    { "ft_memcpy",  "void *ft_memcpy(void *dst, const void *src, size_t n)", CAT_MEMORY, TIER_A, { NULL } },
    { "ft_memmove", "void *ft_memmove(void *dst, const void *src, size_t n)", CAT_MEMORY, TIER_A, { NULL } },
    { "ft_memchr",  "void *ft_memchr(const void *s, int c, size_t n)", CAT_MEMORY, TIER_A, { NULL } },
    { "ft_memcmp",  "int ft_memcmp(const void *s1, const void *s2, size_t n)", CAT_MEMORY, TIER_A, { NULL } },

    /* convert (2) - Tier A */
    { "ft_atoi", "int ft_atoi(const char *nptr)", CAT_CONVERT, TIER_A, { NULL } },
    { "ft_itoa", "char *ft_itoa(int n)", CAT_CONVERT, TIER_A, { "malloc", NULL } },

    /* alloc (8) - Tier A (behavior), Tier B for some conventions */
    { "ft_calloc",  "void *ft_calloc(size_t count, size_t size)", CAT_ALLOC, TIER_A, { "malloc", NULL } },
    { "ft_strdup",  "char *ft_strdup(const char *s)", CAT_ALLOC, TIER_A, { "malloc", NULL } },
    { "ft_substr",  "char *ft_substr(const char *s, unsigned int start, size_t len)", CAT_ALLOC, TIER_A, { "malloc", NULL } },
    { "ft_strjoin", "char *ft_strjoin(const char *s1, const char *s2)", CAT_ALLOC, TIER_A, { "malloc", NULL } },
    { "ft_strtrim", "char *ft_strtrim(const char *s1, const char *set)", CAT_ALLOC, TIER_A, { "malloc", NULL } },
    { "ft_split",   "char **ft_split(const char *s, char c)", CAT_ALLOC, TIER_A, { "malloc", "free", NULL } },
    { "ft_strmapi", "char *ft_strmapi(const char *s, char (*f)(unsigned int, char))", CAT_ALLOC, TIER_A, { "malloc", NULL } },
    { "ft_striteri","void ft_striteri(char *s, void (*f)(unsigned int, char *))", CAT_ALLOC, TIER_A, { NULL } },

    /* output (4) - Tier A */
    { "ft_putchar_fd", "void ft_putchar_fd(char c, int fd)", CAT_OUTPUT, TIER_A, { "write", NULL } },
    { "ft_putstr_fd",  "void ft_putstr_fd(char *s, int fd)", CAT_OUTPUT, TIER_A, { "write", NULL } },
    { "ft_putendl_fd", "void ft_putendl_fd(char *s, int fd)", CAT_OUTPUT, TIER_A, { "write", NULL } },
    { "ft_putnbr_fd",  "void ft_putnbr_fd(int n, int fd)", CAT_OUTPUT, TIER_A, { "write", "free", NULL } },

    /* list (9) - Tier A */
    { "ft_lstnew",     "t_list *ft_lstnew(void *content)", CAT_LIST, TIER_A, { "malloc", NULL } },
    { "ft_lstadd_front","void ft_lstadd_front(t_list **lst, t_list *new_node)", CAT_LIST, TIER_A, { NULL } },
    { "ft_lstsize",    "unsigned int ft_lstsize(t_list *lst)", CAT_LIST, TIER_A, { NULL } },
    { "ft_lstlast",    "t_list *ft_lstlast(t_list *lst)", CAT_LIST, TIER_A, { NULL } },
    { "ft_lstadd_back", "void ft_lstadd_back(t_list **lst, t_list *new_node)", CAT_LIST, TIER_A, { NULL } },
    { "ft_lstdelone",  "void ft_lstdelone(t_list *lst, void (*del)(void *))", CAT_LIST, TIER_A, { "free", NULL } },
    { "ft_lstclear",   "void ft_lstclear(t_list **lst, void (*del)(void *))", CAT_LIST, TIER_A, { "free", NULL } },
    { "ft_lstiter",    "void ft_lstiter(t_list *lst, void (*f)(void *))", CAT_LIST, TIER_A, { NULL } },
    { "ft_lstmap",     "t_list *ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))", CAT_LIST, TIER_A, { "malloc", "free", NULL } },
};

/* Number of entries in the spec table */
static const size_t g_spec_count = sizeof(g_spec_table) / sizeof(g_spec_table[0]);

/* Lookup a spec entry by function name (e.g. "ft_strlen") */
const SpecEntry *spec_find(const char *name)
{
    if (!name)
        return NULL;
    for (size_t i = 0; i < g_spec_count; i++) {
        if (strcmp(g_spec_table[i].name, name) == 0)
            return &g_spec_table[i];
    }
    return NULL;
}

/* Return total number of spec entries (43) */
size_t spec_count(void)
{
    return g_spec_count;
}

/* Return pointer to the full spec table array */
const SpecEntry *spec_all(void)
{
    return g_spec_table;
}

/* Helper: category name for reporting */
const char *spec_category_name(SpecCategory cat)
{
    static const char *names[] = {
        "ctype", "string", "memory", "convert", "alloc", "output", "list"
    };
    if (cat < CAT_COUNT)
        return names[cat];
    return "unknown";
}

/* Helper: tier name for reporting */
const char *spec_tier_name(SpecTier tier)
{
    static const char *names[] = { "A", "B", "C" };
    if (tier <= TIER_C)
        return names[tier];
    return "?";
}