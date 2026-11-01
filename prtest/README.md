# prtest

A pure-C test harness for the 42 **libft** project (subject v19.3).

- Orchestrator (`prtest/`) and harness (`harness/`) are both C.
- macOS dyld interposition for malloc/free/calloc/realloc from day one,
  with a GNU ld `--wrap` fallback for Linux.
- Tests are readable hand-written C in `harness/libft/t_*.c` — a student
  can open them and see exactly what ran.

See `prtest/FILES.md` for the ownership map of this skeleton.
