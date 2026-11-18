"""ft_printf project test suite.

Structure and functional checks follow the ft_printf subject (v12.1):
  - ft_printf(const char *, ...) with header named ft_printf.h
  - Conversions: c s p d i u x X %
  - Makefile with NAME, all, clean, fclean, re rules, -Wall -Wextra -Werror, cc
  - libftprintf.a created with `ar` at the root of the repository
  - Optional libft/ folder: sources + its own Makefile, built by the project Makefile
  - Bonuses in _bonus.{c/h} files, built through a `bonus` rule

Functional tests compare ft_printf() output and return value against the real
printf() for a battery of cases covering every mandatory conversion.
"""

import re
from pathlib import Path

from base import ProjectTest


class FtPrintfProject(ProjectTest):
    name = "ft_printf"
    description = "42 ft_printf project (recode printf, conversions cspdiuxX%)"

    test_file = "tests/run_tests.sh"
    build_cmd = "make"
    test_cmd = "./tests/run_tests.sh normal"

    header_name = "ft_printf.h"
    default_lib_name = "libftprintf.a"

    # ------------------------------------------------------------------ #
    # Structure verification (per subject)
    # ------------------------------------------------------------------ #

    def verify_structure(self, path: Path) -> bool:
        print("\n🔍 Verifying ft_printf project structure...")
        ok = True

        # CHECK 1: ft_printf.h at root or in a subdirectory (subject allows */*.h)
        header = path / self.header_name
        if header.exists():
            print(f"   ✓ {self.header_name} found at project root")
        else:
            matches = [p for p in path.glob(f"*/{self.header_name}")
                       if p.parent.name != "tests"]
            if matches:
                print(f"   ✓ {self.header_name} found in {matches[0].parent.name}/")
            else:
                print(f"❌ ERROR: {self.header_name} not found")
                print("   Subject requires a header named ft_printf.h containing")
                print("   the prototype: int ft_printf(const char *, ...);")
                ok = False

        # CHECK 2: C sources at root or one level deep
        srcs = [s for s in list(path.glob("*.c")) + list(path.glob("*/*.c"))
                if s.parent.name != "tests"]
        if not srcs:
            print("❌ ERROR: No .c source files found")
            ok = False
        else:
            print(f"   ✓ {len(srcs)} source file(s) found")

        # CHECK 3: Makefile at project root with required rules and flags
        makefile = path / "Makefile"
        content = ""
        if not makefile.exists():
            print(f"❌ ERROR: Makefile not found at {path}")
            print("   Required at project root (per subject requirements)")
            ok = False
        else:
            content = makefile.read_text()
            missing_rules = [rule for rule in ["all", "clean", "fclean", "re"]
                             if not re.search(rf"^{re.escape(rule)}\s*:",
                                              content, re.MULTILINE)]
            if "NAME" not in content:
                missing_rules.append("NAME variable")
            if missing_rules:
                print("⚠️  WARNING: Makefile missing: "
                      f"{', '.join(missing_rules)} "
                      "(subject requires NAME, all, clean, fclean, re)")
            if not all(flag in content for flag in ["-Wall", "-Wextra", "-Werror"]):
                print("⚠️  WARNING: Makefile doesn't use -Wall -Wextra -Werror")
                print("   Subject requires compilation with these flags using cc")
            if "libtool" in content.lower():
                print("⚠️  WARNING: Makefile mentions libtool - forbidden by subject,")
                print("   the library must be created with the ar command")
            print("   ✓ Makefile found at project root")

        # CHECK 4: optional libft/ folder (sources + its own Makefile)
        libft_dir = path / "libft"
        if libft_dir.exists():
            has_sources = any(libft_dir.glob("*.c")) or any(libft_dir.glob("*/*.c"))
            has_makefile = (libft_dir / "Makefile").exists()
            if has_sources and has_makefile:
                print("   ✓ libft/ folder found with sources and its Makefile")
            else:
                print("⚠️  WARNING: libft/ folder found but is missing sources or its Makefile")
                print("   Subject: copy libft sources and its associated Makefile")
                print("   into a libft folder, and compile the library from your Makefile")

        # CHECK 5: bonus files require a bonus rule
        bonus_files = list(path.glob("*_bonus.c")) + list(path.glob("*_bonus.h"))
        if bonus_files and not re.search(r"^bonus\s*:", content, re.MULTILINE):
            print("⚠️  WARNING: _bonus files found but Makefile has no 'bonus' rule")
            print("   Subject: bonuses must be built through a bonus rule,")
            print("   and evaluated separately from the mandatory part")

        if ok:
            print("\n✅ ft_printf project structure verification PASSED!")
            print("   - ft_printf.h header ✓")
            print("   - C source files ✓")
            print("   - Makefile at project root ✓")
        return ok

    # ------------------------------------------------------------------ #
    # Build artifact verification
    # ------------------------------------------------------------------ #

    def library_name(self, path: Path) -> str:
        makefile = path / "Makefile"
        if makefile.exists():
            match = re.search(r"^NAME\s*[?:+]?=\s*(\S+)",
                              makefile.read_text(), re.MULTILINE)
            if match:
                return match.group(1)
        return self.default_lib_name

    def post_build_check(self, path: Path) -> bool:
        lib = path / self.library_name(path)
        if lib.exists():
            print(f"✅ {lib.name} created at project root")
            return True
        print(f"❌ ERROR: {self.default_lib_name} was not created at the project root")
        print("   Subject: 'Your libftprintf.a has to be created at the root of")
        print("   your repository' using the ar command (libtool is forbidden)")
        return False

    def build_troubleshooting(self):
        return ["Check if 'gcc' is available",
                "Verify Makefile syntax (rules: NAME, all, clean, fclean, re)",
                "Ensure ft_printf.c defines ft_printf(const char *, ...)",
                "If using libft/, ensure its Makefile builds libft.a successfully"]

    # ------------------------------------------------------------------ #
    # Install templates
    # ------------------------------------------------------------------ #

    def install_infrastructure(self) -> bool:
        print(f"Installing test infrastructure for {self.name}")
        print("📁 Creating ft_printf project structure (per en.subject.pdf):")

        self.write_if_missing(self.header_name, FT_PRINTF_H)
        self.write_if_missing("ft_printf.c", FT_PRINTF_C_STUB)
        self.write_if_missing("Makefile", FT_PRINTF_MAKEFILE)
        self.write_if_missing("tests/run_tests.sh", RUN_TESTS_SH, executable=True)
        self.write_if_missing("tests/main.c", TEST_MAIN_C)

        print("\n✅ ft_printf test infrastructure installed successfully!")
        print("   - ft_printf.h header ✓")
        print("   - ft_printf.c placeholder (replace with your implementation) ✓")
        print("   - Makefile at project root ✓")
        print("   - tests/ directory with comparison test runner ✓")
        print("\n📝 Project now ready for testing with './testme ft_printf .'")
        return True


# ---------------------------------------------------------------------- #
# Template files created by `testme install ft_printf`
# ---------------------------------------------------------------------- #

FT_PRINTF_H = """#ifndef FT_PRINTF_H
# define FT_PRINTF_H

# include <stdarg.h>

int	ft_printf(const char *format, ...);

#endif
"""

FT_PRINTF_C_STUB = """/* TODO: replace this placeholder with your own ft_printf implementation. */
#include "ft_printf.h"
#include <unistd.h>

int	ft_printf(const char *format, ...)
{
	(void)format;
	write(1, "ft_printf: not implemented yet\\n", 31);
	return (0);
}
"""

FT_PRINTF_MAKEFILE = """NAME = libftprintf.a
CC = cc
CFLAGS = -Wall -Wextra -Werror
AR = ar rcs

SRC = ft_printf.c
OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(AR) $(NAME) $(OBJ)

%.o: %.c ft_printf.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

bonus:
	$(AR) $(NAME) $(OBJ)

.PHONY: all clean fclean re bonus
"""

RUN_TESTS_SH = r"""#!/bin/bash
# Test runner for ft_printf project
# Usage: ./tests/run_tests.sh [normal|asan]
#
# Compares ft_printf() against the real printf() for output bytes and
# return values across all mandatory conversions: c s p d i u x X %

MODE="${1:-normal}"

# Build the library
make fclean >/dev/null 2>&1
if ! make >/tmp/ft_printf_build.log 2>&1; then
    echo "❌ Build failed:"
    cat /tmp/ft_printf_build.log
    exit 1
fi

# Locate the library archive (use NAME from the Makefile, fallback libftprintf.a)
LIB="libftprintf.a"
NAME_VAR=$(grep -E '^NAME[[:space:]]*[?:+]?=' Makefile | head -n1 | sed 's/^[^=]*=[[:space:]]*//')
if [ -n "$NAME_VAR" ] && [ -f "$NAME_VAR" ]; then
    LIB="$NAME_VAR"
fi

# Locate the header directory (root or a subdirectory)
HDR_DIR="."
if [ ! -f ft_printf.h ]; then
    FOUND=$(find . -name ft_printf.h -not -path './tests/*' | head -n1)
    [ -n "$FOUND" ] && HDR_DIR=$(dirname "$FOUND")
fi

# -Wno-format-security: the harness legitimately tests a non-literal empty format
FLAGS="-Wall -Wextra -Werror -Wno-format-security -I. -I$HDR_DIR"
if [ "$MODE" = "asan" ]; then
    FLAGS="$FLAGS -g -fsanitize=address,leak"
    echo "🧪 Running with AddressSanitizer/LeakSanitizer"
fi

if ! gcc $FLAGS tests/main.c "$LIB" -o /tmp/ft_printf_test_bin 2>/tmp/ft_printf_cc.log; then
    echo "❌ Test compilation failed:"
    cat /tmp/ft_printf_cc.log
    exit 1
fi

/tmp/ft_printf_test_bin 2>&1 | tee /tmp/ft_printf_asan.log
STATUS=${PIPESTATUS[0]}

# LeakSanitizer needs ptrace; in restricted environments it aborts fatally.
# Fall back to plain AddressSanitizer so the functional tests still run.
if [ "$MODE" = "asan" ] && [ $STATUS -ne 0 ] && \
   grep -q "LeakSanitizer has encountered a fatal error" /tmp/ft_printf_asan.log; then
    echo "⚠️  LeakSanitizer unavailable in this environment - rerunning without leak detection"
    ASAN_OPTIONS=detect_leaks=0 /tmp/ft_printf_test_bin
    STATUS=$?
fi

rm -f /tmp/ft_printf_test_bin /tmp/ft_printf_asan.log
make fclean >/dev/null 2>&1
exit $STATUS
"""

TEST_MAIN_C = r"""/* ft_printf test harness - differential testing against the real printf().
 * Every case runs both functions with identical arguments, redirecting stdout
 * to a temp file each time, then compares output bytes and return values.
 */
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "ft_printf.h"

# define BUF_SIZE 8192

static int g_total = 0;
static int g_passed = 0;
static char g_marker;   /* used as a real %p target */

/* routed through variables so gcc cannot warn on the edge cases below:
 * - zero-length literal format (-Wformat-zero-length)
 * - literal NULL for %s (-Wformat-overflow) - calling with NULL is UB
 *   in C, but glibc prints "(null)" and many ft_printf do the same
 */
static const char *volatile g_empty_fmt = "";
static char *volatile g_null_str = NULL;

static int read_all(FILE *f, char *buf)
{
	long len;

	fflush(stdout);
	rewind(f);
	len = (long)fread(buf, 1, BUF_SIZE - 1, f);
	buf[len] = '\0';
	return ((int)len);
}

#define CAPTURE(fn, ret_var, buf_var, len_var, ...)          \
	do {                                                     \
		FILE *tmp_ = tmpfile();                              \
		int saved_ = dup(STDOUT_FILENO);                     \
		int ret_;                                            \
		fflush(stdout);                                      \
		dup2(fileno(tmp_), STDOUT_FILENO);                   \
		ret_ = fn(__VA_ARGS__);                              \
		fflush(stdout);                                      \
		dup2(saved_, STDOUT_FILENO);                         \
		close(saved_);                                       \
		ret_var = ret_;                                      \
		len_var = read_all(tmp_, buf_var);                   \
		fclose(tmp_);                                        \
	} while (0)

static void compare(const char *label, int line, int strict,
					int ft_ret, const char *ft_buf, int ft_len,
					int pf_ret, const char *pf_buf, int pf_len)
{
	int ok;

	g_total++;
	ok = (ft_ret == pf_ret) && (ft_len == pf_len)
		&& (pf_len == 0 || memcmp(ft_buf, pf_buf, (size_t)pf_len) == 0);
	if (ok)
	{
		g_passed++;
		printf("✅ [%s] ok\n", label);
	}
	else if (!strict)
	{
		printf("⚠️  [%s] (line %d) differs - allowed: undefined behavior case\n",
			label, line);
		printf("   ft_printf : ret=%d out=[%.*s]\n", ft_ret, ft_len, ft_buf);
		printf("   printf    : ret=%d out=[%.*s]\n", pf_ret, pf_len, pf_buf);
	}
	else
	{
		printf("❌ [%s] (line %d)\n", label, line);
		printf("   ft_printf : ret=%d out=[%.*s]\n", ft_ret, ft_len, ft_buf);
		printf("   printf    : ret=%d out=[%.*s]\n", pf_ret, pf_len, pf_buf);
	}
}

#define CHECK(label, ...)                                     \
	do {                                                      \
		char ft_buf_[BUF_SIZE]; char pf_buf_[BUF_SIZE];       \
		int ft_ret_, pf_ret_, ft_len_, pf_len_;               \
		CAPTURE(ft_printf, ft_ret_, ft_buf_, ft_len_, __VA_ARGS__); \
		CAPTURE(printf, pf_ret_, pf_buf_, pf_len_, __VA_ARGS__); \
		compare(label, __LINE__, 1,                           \
			ft_ret_, ft_buf_, ft_len_, pf_ret_, pf_buf_, pf_len_); \
	} while (0)

#define CHECK_SOFT(label, ...)                                \
	do {                                                      \
		char ft_buf_[BUF_SIZE]; char pf_buf_[BUF_SIZE];       \
		int ft_ret_, pf_ret_, ft_len_, pf_len_;               \
		CAPTURE(ft_printf, ft_ret_, ft_buf_, ft_len_, __VA_ARGS__); \
		CAPTURE(printf, pf_ret_, pf_buf_, pf_len_, __VA_ARGS__); \
		compare(label, __LINE__, 0,                           \
			ft_ret_, ft_buf_, ft_len_, pf_ret_, pf_buf_, pf_len_); \
	} while (0)

int main(void)
{
	printf("🧪 ft_printf vs printf comparison tests\n\n");

	/* Literals / %% */
	CHECK("no-format string", "Hello, World!\n");
	CHECK("empty format", g_empty_fmt);
	CHECK("percent only", "%%");
	CHECK("multiple percent", "%%%%%%");

	/* %c */
	CHECK("c basic", "%c", 'A');
	CHECK("c space", "%c", ' ');
	CHECK("c multiple", "%c%c%c", 'a', 'b', 'c');

	/* %s */
	CHECK("s basic", "%s", "42 school");
	CHECK("s empty", "%s", "");
	CHECK("s long", "%s",
		"A longer string to exercise the output path - 0123456789 0123456789 0123456789");

	/* %d / %i */
	CHECK("d zero", "%d", 0);
	CHECK("d positive", "%d", 42);
	CHECK("d negative", "%d", -42);
	CHECK("d int max", "%d", INT_MAX);
	CHECK("d int min", "%d", INT_MIN);
	CHECK("i positive", "%i", 12345);
	CHECK("i negative", "%i", -12345);

	/* %u */
	CHECK("u zero", "%u", 0u);
	CHECK("u positive", "%u", 42u);
	CHECK("u uint max", "%u", UINT_MAX);

	/* %x / %X */
	CHECK("x zero", "%x", 0);
	CHECK("x positive", "%x", 0x1a2b3c);
	CHECK("x negative", "%x", -1);
	CHECK("x int min", "%x", INT_MIN);
	CHECK("X zero", "%X", 0);
	CHECK("X positive", "%X", 0xABCDEF);
	CHECK("X negative", "%X", -1);

	/* %p */
	CHECK("p null", "%p", (void *)NULL);
	CHECK("p data ptr", "%p", (void *)&g_marker);
	CHECK("p func ptr", "%p", (void *)&main);

	/* Mixed */
	CHECK("d and s", "%s is %d years old", "Bob", 25);
	CHECK("multiple convs", "%d %d %d", 1, -2, 3);
	CHECK("consecutive", "%d%d%d%d", 1, 22, 333, 4444);
	CHECK("text around convs", "a%c b%s c%d", 'X', "Y", 42);
	CHECK("all convs", "%c %s %p %d %i %u %x %X %%", 'Q', "mix",
		(void *)&g_marker, -123, 456, 789u, 0xdead, 0xBEEF);

	/* %s with NULL is undefined behavior in C - soft check, warn only */
	CHECK_SOFT("s null (UB, glibc prints \"(null)\")", "%s", g_null_str);

	printf("\n=== ft_printf test results: %d/%d passed ===\n", g_passed, g_total);
	if (g_passed == g_total)
	{
		printf("🎉 All tests passed!\n");
		return (0);
	}
	printf("💥 Some tests failed - compare your output with printf's above.\n");
	return (1);
}
"""
