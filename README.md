# TestMe - Student Project Testing Framework

A CLI tool for testing student projects (libft, etc.) with automated structure verification, build validation, and test execution.

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

# Test a specific path
testme libft /path/to/libft-project

# List available project types
testme list

# Install test infrastructure for a new project
testme install libft
```

## Features

- ✅ **Structure Verification**: Checks libft/ folder, Makefile, tests/ directory
- ✅ **Build Validation**: Runs `make` and verifies compilation
- ✅ **Test Execution**: Runs `./tests/run_tests.sh` with clear output
- ✅ **Error Diagnostics**: Provides specific feedback on structure issues
- ✅ **CI/CD Ready**: Can be integrated into automated testing workflows

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
