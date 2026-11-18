# TestMe - Student Project Testing Framework

A CLI tool for testing student projects (libft, ft_printf, etc.) with automated structure verification, build validation, and test execution.

Each supported project has its own test suite in a **separate file** under `projects/`:

```
projects/
├── base.py          # Shared ProjectTest base class (build/test flow)
├── libft.py         # libft structure checks and install templates
└── ft_printf.py     # ft_printf structure checks, install templates and
                     # differential tests against the real printf()
```

Adding support for a new project means adding one new file in `projects/` that
defines a `ProjectTest` subclass - the CLI discovers it automatically.

## Installation

### Quick Install (One Command)

```bash
curl -sSL https://raw.githubusercontent.com/AyoubMzian-Dev/testme/main/install.sh | bash
```

### Manual Install

```bash
# Clone the repository
git clone https://github.com/AyoubMzian-Dev/testme.git
cd testme

# Make executable
chmod +x testme

# Add to PATH
export PATH="$PWD:$PATH"
```

## Usage

```bash
# Test a libft project
testme libft .

# Test an ft_printf project
testme ft_printf /path/to/ft_printf-project

# List available project types
testme list

# Install test infrastructure for a new project
testme install libft
testme install ft_printf
```

## Features

- ✅ **Structure Verification**: Checks required files, Makefile rules and flags per subject
- ✅ **Build Validation**: Runs `make` and verifies compilation
- ✅ **Test Execution**: Runs `./tests/run_tests.sh` with clear output
- ✅ **Error Diagnostics**: Provides specific feedback on structure issues
- ✅ **CI/CD Ready**: Can be integrated into automated testing workflows

## ft_printf testing (per en.subject.pdf)

`testme ft_printf <path>` verifies:

- `ft_printf.h` header containing the `ft_printf(const char *, ...)` prototype
- Makefile at project root with `NAME`, `all`, `clean`, `fclean`, `re` rules,
  compiled with `cc -Wall -Wextra -Werror`
- `libftprintf.a` created with `ar` at the root of the repository (libtool is flagged as forbidden)
- Optional `libft/` folder: sources + its own Makefile
- Bonus files (`*_bonus.{c,h}`) require a `bonus` rule in the Makefile

After `testme install ft_printf`, `tests/run_tests.sh` runs a differential
harness (`tests/main.c`) that compares `ft_printf()` against the real `printf()`
for **output bytes and return value** across 36 cases covering every mandatory
conversion (`c s p d i u x X %`), including `INT_MIN`/`INT_MAX`, `UINT_MAX`,
negative hex, NULL `%p`, `(nil)`, mixed formats, and a soft (warn-only) check
for the undefined-behavior `%s` NULL case. Run it in ASan mode with:

```bash
./tests/run_tests.sh asan
```

## Project Structure (per subject.md)

According to the libft subject, your project must have:

```
project_root/
├── Makefile                    # Project Makefile
├── libft/                      # libft library folder
│   ├── libft.h                # Library header with all prototypes
│   ├── src/                   # Source files
│   │   ├── ft_isalpha.c       (✓ done)
│   │   ├── ft_isdigit.c       (✓ done)
│   │   ├── ...
│   │   ├── ft_lstmap.c        (✓ done)
│   │   └── ft_itoa.c          (✓ done)
│   └── libft.a                # Compiled library
└── tests/                     # Test files
    ├── main.c
    └── run_tests.sh
```

## License

MIT License - see LICENSE file for details
