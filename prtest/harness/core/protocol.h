#ifndef PRTEST_PROTOCOL_H
#define PRTEST_PROTOCOL_H
#include <stddef.h>

typedef enum { V_PASS=0, V_FAIL, V_CRASH, V_TIMEOUT, V_LEAK, V_MEMORY, V_WARN } Verdict;

/* ---- test registry (auto-registration via constructor) ---- */
void reg_test(const char *name, void (*fn)(void), const char *file);
#define TEST_CONCAT_(a, b) a ## b
#define TEST_CONCAT(a, b) TEST_CONCAT_(a, b)
/* Usage: define `static void my_test(void){...}` then `TEST("name", my_test);`\n   The constructor registers the already-defined function at load time. */
#define TEST(name, fn) \
    __attribute__((constructor)) static void _reg_##fn(void) { \
        reg_test(name, fn, __FILE__); \
    }

/* ---- expectations (set a failure flag; child exits non-zero on fail) ---- */
void expect_true(const char *expr, int cond, const char *file, int line);
void expect_eq_int(const char *expr, long got, long want, const char *file, int line);
void expect_ptr_in(const char *expr, const void *got, const void *lo, const void *hi, const char *file, int line);
void expect_mem_eq(const char *expr, const void *got, const void *want, size_t n, const char *file, int line);
void fail(const char *msg, const char *file, int line);

/* ---- guard-page buffers ---- */
typedef struct { char *base; size_t n; } Gbuf;
Gbuf gbuf_new(size_t n);
void gbuf_free(Gbuf g);
char *gbuf_at(Gbuf g, size_t i);

/* ---- instrumented allocator ---- */
void awrap_install(void);
void awrap_reset(void);
size_t awrap_live_blocks(void);
size_t awrap_live_bytes(void);
void   awrap_fail_at(size_t n);

/* ---- deterministic RNG ---- */
void rng_seed(unsigned long long s);
unsigned long long rng_next(void);

/* ---- result reporting to fd 3 ---- */
void send_result(const char *func, const char *test, Verdict v, const char *detail);

#endif