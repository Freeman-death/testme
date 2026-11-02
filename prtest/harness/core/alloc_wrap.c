#include "protocol.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

#define GUARD_SIZE 4096
#define POISON_ALLOC 0xA5
#define POISON_FREE  0x5A

typedef struct BlockInfo {
    void *ptr;
    size_t size;
    size_t guard_page;
    int freed;
} BlockInfo;

static size_t g_live_blocks = 0;
static size_t g_live_bytes = 0;
static size_t g_calls = 0;
static size_t g_fail_at = 0;
static BlockInfo *g_blocks = NULL;
static size_t g_nblocks = 0;
static size_t g_cap = 0;

void awrap_install(void) {
    g_live_blocks = g_live_bytes = g_calls = g_fail_at = 0;
    g_nblocks = 0;
}

void awrap_reset(void) {
    for (size_t i = 0; i < g_nblocks; i++) {
        if (!g_blocks[i].freed && g_blocks[i].ptr) {
            munmap((char *)g_blocks[i].ptr - GUARD_SIZE, g_blocks[i].size + 2 * GUARD_SIZE);
        }
    }
    g_live_blocks = g_live_bytes = g_nblocks = 0;
}

void awrap_fail_at(size_t n) { g_fail_at = n; }
size_t awrap_live_blocks(void) { return g_live_blocks; }
size_t awrap_live_bytes(void)  { return g_live_bytes; }

static void track(void *user_ptr, size_t size, void *alloc_base) {
    if (g_nblocks >= g_cap) {
        g_cap = g_cap ? g_cap * 2 : 256;
        g_blocks = realloc(g_blocks, g_cap * sizeof(BlockInfo));
    }
    g_blocks[g_nblocks++] = (BlockInfo){user_ptr, size, (size_t)alloc_base, 0};
    g_live_blocks++;
    g_live_bytes += size;
}

static int find_block(void *ptr) {
    for (size_t i = 0; i < g_nblocks; i++) {
        if (g_blocks[i].ptr == ptr) return (int)i;
    }
    return -1;
}

static void *alloc_with_guards(size_t n) {
    size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
    size_t aligned_size = (n + page_size - 1) & ~(page_size - 1);
    size_t total = aligned_size + 2 * page_size;
    
    void *base = mmap(NULL, total, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED) return NULL;

    mprotect(base, page_size, PROT_NONE);
    mprotect((char *)base + page_size + aligned_size, page_size, PROT_NONE);
    
    void *user_ptr = (char *)base + page_size;
    memset(user_ptr, POISON_ALLOC, n);
    
    track(user_ptr, n, base);
    return user_ptr;
}

static void *w_malloc(size_t n) {
    if (n == 0) n = 1;
    if (++g_calls == g_fail_at) return NULL;
    return alloc_with_guards(n);
}

static void w_free(void *ptr) {
    if (!ptr) return;
    int idx = find_block(ptr);
    if (idx < 0) {
        fprintf(stderr, "awrap: invalid free of %p\n", ptr);
        abort();
    }
    if (g_blocks[idx].freed) {
        fprintf(stderr, "awrap: double free of %p\n", ptr);
        abort();
    }
    
    size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
    size_t aligned_size = (g_blocks[idx].size + page_size - 1) & ~(page_size - 1);
    void *base = (char *)ptr - page_size;
    
    memset(ptr, POISON_FREE, g_blocks[idx].size);
    mprotect(base, aligned_size + 2 * page_size, PROT_NONE);
    
    g_live_bytes -= g_blocks[idx].size;
    g_live_blocks--;
    g_blocks[idx].freed = 1;
}

static void *w_calloc(size_t a, size_t b) {
    if (a && b > SIZE_MAX / a) return NULL;
    size_t n = a * b;
    void *p = w_malloc(n);
    if (p) memset(p, 0, n);
    return p;
}

static void *w_realloc(void *ptr, size_t n) {
    if (!ptr) return w_malloc(n);
    if (n == 0) { w_free(ptr); return NULL; }
    
    int idx = find_block(ptr);
    if (idx < 0) {
        fprintf(stderr, "awrap: realloc of invalid pointer %p\n", ptr);
        abort();
    }
    if (g_blocks[idx].freed) {
        fprintf(stderr, "awrap: realloc of freed pointer %p\n", ptr);
        abort();
    }
    
    void *newp = w_malloc(n);
    if (newp) {
        size_t copy = g_blocks[idx].size < n ? g_blocks[idx].size : n;
        memcpy(newp, ptr, copy);
        w_free(ptr);
    }
    return newp;
}

#if defined(__APPLE__)
static void *w_malloc_sym(size_t n) { return w_malloc(n); }
static void  w_free_sym(void *p) { w_free(p); }
static void *w_calloc_sym(size_t a, size_t b) { return w_calloc(a, b); }
static void *w_realloc_sym(void *b, size_t n) { return w_realloc(b, n); }

__attribute__((used, section("__DATA,__interpose")))
static const struct { const void *replacement; const void *replacee; } _interpose[] = {
    { (const void *)w_malloc_sym,  (const void *)malloc  },
    { (const void *)w_free_sym,    (const void *)free    },
    { (const void *)w_calloc_sym,  (const void *)calloc  },
    { (const void *)w_realloc_sym, (const void *)realloc },
};
#else
#define w_malloc  __wrap_malloc
#define w_free    __wrap_free
#define w_calloc  __wrap_calloc
#define w_realloc __wrap_realloc
#endif