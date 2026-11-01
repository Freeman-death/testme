#include "protocol.h"
#include <sys/mman.h>
#include <unistd.h>

static size_t page(void) { return (size_t)sysconf(_SC_PAGESIZE); }

Gbuf gbuf_new(size_t n) {
    Gbuf g; g.n = n;
    size_t pg = page();
    size_t total = pg * 3 + n;
    char *p = mmap(NULL, total, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) { g.base = NULL; return g; }
    mprotect(p, pg, PROT_NONE);
    mprotect(p + pg + n, pg, PROT_NONE);
    g.base = p + pg;
    return g;
}

void gbuf_free(Gbuf g) {
    if (!g.base) return;
    size_t pg = page();
    munmap(g.base - pg, pg * 3 + g.n);
}

char *gbuf_at(Gbuf g, size_t i) { return g.base + i; }
