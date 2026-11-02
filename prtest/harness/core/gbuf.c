#include "protocol.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CANARY_SIZE 16
static const unsigned char CANARY_PATTERN[16] = {
    0xDE, 0xAD, 0xC0, 0xDE, 0xBE, 0xEF, 0xFE, 0xED,
    0xBA, 0xAD, 0xF0, 0x0D, 0xC0, 0xFF, 0xEE, 0x00
};

static size_t page_size(void) { return (size_t)sysconf(_SC_PAGESIZE); }

Gbuf gbuf_new(size_t n) {
    Gbuf g = {0};
    g.n = n;
    size_t pg = page_size();
    
    size_t payload = n + 2 * CANARY_SIZE;
    size_t aligned = (payload + pg - 1) & ~(pg - 1);
    size_t total = aligned + 2 * pg;
    
    char *base = mmap(NULL, total, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED) return g;
    
    mprotect(base, pg, PROT_NONE);
    mprotect(base + pg + aligned, pg, PROT_NONE);
    
    char *user_base = base + pg;
    
    memcpy(user_base, CANARY_PATTERN, CANARY_SIZE);
    memcpy(user_base + CANARY_SIZE + n, CANARY_PATTERN, CANARY_SIZE);
    
    g.base = user_base + CANARY_SIZE;
    return g;
}

void gbuf_free(Gbuf g) {
    if (!g.base) return;
    size_t pg = page_size();
    char *base = g.base - CANARY_SIZE - pg;
    
    for (size_t i = 0; i < CANARY_SIZE; i++) {
        unsigned char before = g.base[-CANARY_SIZE + i];
        unsigned char after = g.base[g.n + i];
        if (before != CANARY_PATTERN[i] || after != CANARY_PATTERN[i]) {
            fprintf(stderr, "gbuf: canary corrupted at index %zu (expected %02x, got %02x/%02x)\n",
                    i, CANARY_PATTERN[i], before, after);
            abort();
        }
    }
    
    size_t payload = g.n + 2 * CANARY_SIZE;
    size_t aligned = (payload + pg - 1) & ~(pg - 1);
    size_t total = aligned + 2 * pg;
    munmap(base, total);
}

char *gbuf_at(Gbuf g, size_t i) {
    if (i >= g.n) {
        fprintf(stderr, "gbuf: index %zu out of bounds (size %zu)\n", i, g.n);
        abort();
    }
    return g.base + i;
}