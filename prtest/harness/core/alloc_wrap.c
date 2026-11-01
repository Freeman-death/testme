#include "protocol.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>

static size_t g_live_blocks, g_live_bytes, g_calls, g_fail_at;
static void *g_blocks[16384];
static size_t g_nblocks;

void awrap_install(void) { g_live_blocks = g_live_bytes = g_calls = g_fail_at = 0; g_nblocks = 0; }
void awrap_reset(void)   { g_live_blocks = g_live_bytes = g_nblocks = 0; }
void awrap_fail_at(size_t n) { g_fail_at = n; }
size_t awrap_live_blocks(void) { return g_live_blocks; }
size_t awrap_live_bytes(void)  { return g_live_bytes; }

static void track(void *p, size_t n) {
    if (g_nblocks < 16384) g_blocks[g_nblocks++] = p;
    g_live_blocks++; g_live_bytes += n;
}
static void untrack(void *p) {
    for (size_t i = 0; i < g_nblocks; i++)
        if (g_blocks[i] == p) { g_blocks[i] = g_blocks[--g_nblocks]; break; }
    g_live_blocks--;
}

#if defined(__APPLE__)
static void *w_malloc(size_t n);
static void  w_free(void *p);
static void *w_calloc(size_t a, size_t b);
static void *w_realloc(void *b, size_t n);
__attribute__((used, section("__DATA,__interpose")))
static const struct { const void *replacement; const void *replacee; } _interpose[] = {
    { (const void *)w_malloc,  (const void *)malloc  },
    { (const void *)w_free,    (const void *)free    },
    { (const void *)w_calloc,  (const void *)calloc  },
    { (const void *)w_realloc, (const void *)realloc },
};
#else
#define w_malloc  __wrap_malloc
#define w_free    __wrap_free
#define w_calloc  __wrap_calloc
#define w_realloc __wrap_realloc
#endif

static void *w_malloc(size_t n) {
    if (++g_calls == g_fail_at) return NULL;
    void *p = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) return NULL;
    track(p, n);
    return p;
}
static void w_free(void *p) {
    if (!p) return;
    untrack(p);
    g_live_bytes -= 0; /* size unknown at free in this stub */
    munmap(p, 4096);
}
static void *w_calloc(size_t a, size_t b) {
    size_t n = a * b;
    void *p = w_malloc(n);
    if (p) memset(p, 0, n);
    return p;
}
static void *w_realloc(void *b, size_t n) {
    if (!b) return w_malloc(n);
    if (!n) { w_free(b); return NULL; }
    void *q = w_malloc(n);
    if (q) { memcpy(q, b, n); w_free(b); }
    return q;
}
