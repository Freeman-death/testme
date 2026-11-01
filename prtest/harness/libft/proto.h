#ifndef PRTEST_LIBFT_PROTO_H
#define PRTEST_LIBFT_PROTO_H
/* Subject-derived prototypes for libft v19.3 (43 functions).
   These are the harness's own declarations — NOT the student's libft.h.
   A separate conformance check compiles the student's header alongside
   these to catch prototype conflicts. Owned/refined by subagent 2. */
#include <stddef.h>

/* ctype (7) */
int ft_isalpha(int c);
int ft_isdigit(int c);
int ft_isalnum(int c);
int ft_isascii(int c);
int ft_isprint(int c);
int ft_toupper(int c);
int ft_tolower(int c);

/* string (7) */
size_t ft_strlen(const char *s);
size_t ft_strlcpy(char *dst, const char *src, size_t size);
size_t ft_strlcat(char *dst, const char *src, size_t size);
char  *ft_strchr(const char *s, int c);
char  *ft_strrchr(const char *s, int c);
char  *ft_strnstr(const char *hay, const char *need, size_t len);
int    ft_strncmp(const char *s1, const char *s2, size_t n);

/* memory (6) */
void *ft_memset(void *s, int c, size_t n);
void  ft_bzero(void *s, size_t n);
void *ft_memcpy(void *dst, const void *src, size_t n);
void *ft_memmove(void *dst, const void *src, size_t n);
void *ft_memchr(const void *s, int c, size_t n);
int   ft_memcmp(const void *s1, const void *s2, size_t n);

/* convert (2) */
int   ft_atoi(const char *nptr);
char *ft_itoa(int n);

/* alloc (8) */
void  *ft_calloc(size_t count, size_t size);
char  *ft_strdup(const char *s);
char  *ft_substr(const char *s, unsigned int start, size_t len);
char  *ft_strjoin(const char *s1, const char *s2);
char  *ft_strtrim(const char *s1, const char *set);
char **ft_split(const char *s, char c);
char  *ft_strmapi(const char *s, char (*f)(unsigned int, char));
void   ft_striteri(char *s, void (*f)(unsigned int, char *));

/* output (4) */
void ft_putchar_fd(char c, int fd);
void ft_putstr_fd(char *s, int fd);
void ft_putendl_fd(char *s, int fd);
void ft_putnbr_fd(int n, int fd);

/* list (9) — mandatory in v19.3 */
typedef struct s_list {
    void *content;
    struct s_list *next;
} t_list;

t_list *ft_lstnew(void *content);
void    ft_lstadd_front(t_list **lst, t_list *new_node);
unsigned int ft_lstsize(t_list *lst);
t_list *ft_lstlast(t_list *lst);
void    ft_lstadd_back(t_list **lst, t_list *new_node);
void    ft_lstdelone(t_list *lst, void (*del)(void *));
void    ft_lstclear(t_list **lst, void (*del)(void *));
void    ft_lstiter(t_list *lst, void (*f)(void *));
t_list *ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *));

#endif
