# File ownership map (prtest-base skeleton)

The base branch ships a compiling skeleton. Each subagent branch owns a
disjoint set of files (see below). New `harness/libft/t_*.c` files
auto-register through the `TEST()` macro + constructor registry in
`harness/core/runner.c`, so test-file authors never touch `main.c`.

## Orchestrator (prtest binary) — `prtest/src/`
- `main.c`      CLI dispatch            — subagent 1 (orchestrator)
- `detect.c`    project detection       — subagent 1
- `sandbox.c`   copy + build            — subagent 1
- `proc.c`      fork/exec/timeout       — subagent 1
- `report.c`    terminal report         — subagent 1
- `results.c`   child protocol parser   — subagent 1
- `spec.c`      43-function spec table  — subagent 2 (spec/static)
- `static.c`    nm/header/Makefile checks — subagent 2

## Harness (compiled by orchestrator at runtime) — `prtest/harness/`
- `core/protocol.h`  public harness API (TEST, expect_*, gbuf, awrap, rng, send_result)
- `core/runner.c`    fork-per-test loop + registry + send_result + main() — subagent 1
- `core/gbuf.c`      guard-page buffers                            — subagent 1
- `core/alloc_wrap.c` malloc interpose + leak table + injection     — subagent 1
- `core/expect.c`    expectation helpers                           — subagent 1
- `core/rng.c`       splitmix64                                    — subagent 1
- `libft/proto.h`    subject-derived prototypes                    — subagent 2
- `libft/main.c`     harness main (iterates registry)              — subagent 1
- `libft/t_ctype.c`  ctype tests                                   — subagent 1
- `libft/t_string.c` string tests                                  — subagent 3
- `libft/t_memory.c` memory tests                                  — subagent 3
- `libft/t_convert.c` convert (itoa) tests                         — subagent 3
- `libft/t_alloc.c`  substr/strjoin/strtrim/split/strmapi/striteri — subagent 4
- `libft/t_output.c` put*_fd tests                                 — subagent 4
- `libft/t_list.c`   list family tests                             — subagent 4
- `libft/ref/`       oracles                                       — subagent 3
