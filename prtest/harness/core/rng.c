#include "protocol.h"

static unsigned long long g_state;

void rng_seed(unsigned long long s) { g_state = s ? s : 0x9e3779b97f4a7c15ULL; }
unsigned long long rng_next(void) {
    g_state += 0x9e3779b97f4a7c15ULL;
    unsigned long long z = g_state;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}
