#include "../core/protocol.h"
#include <stdio.h>

/* Harness main: iterate the auto-registered test table. New t_*.c files
   register themselves via TEST(); no edits needed here. */
int main(void) {
    fprintf(stderr, "harness main: enter\n");
    return 0; /* TODO subagent 1: fork-per-test loop (see runner.c) */
}
