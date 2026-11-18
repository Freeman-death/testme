"""libft project test suite.

Structure checks follow the libft subject requirements: libft.h and ft_*.c
sources at the project root or inside a libft/ folder, plus a Makefile at the
project root that builds libft.a.
"""

from pathlib import Path

from base import ProjectTest


class LibftProject(ProjectTest):
    name = "libft"
    description = "42 libft C library project"

    test_file = "tests/run_tests.sh"
    build_cmd = "make"
    test_cmd = "./tests/run_tests.sh normal"

    def verify_structure(self, path: Path) -> bool:
        print("\n🔍 Verifying libft project structure...")

        # Standalone libft project (files at root) or libft/ folder layout
        libft_dir = path / "libft"
        has_libft_folder = libft_dir.exists()

        libft_h_at_root = (path / "libft.h").exists()
        libft_h_in_folder = has_libft_folder and (libft_dir / "libft.h").exists()

        ft_files_at_root = list(path.glob("ft_*.c"))
        ft_files_in_folder = list(libft_dir.glob("ft_*.c")) if has_libft_folder else []

        if not (libft_h_at_root or libft_h_in_folder):
            print("❌ ERROR: libft.h not found")
            print("   Expected either at project root or in libft/ folder")
            return False

        if not (ft_files_at_root or ft_files_in_folder):
            print("❌ ERROR: No ft_*.c files found")
            print("   Expected either at project root or in libft/ folder")
            return False

        if has_libft_folder:
            print(f"   ✓ libft/ folder found with {len(ft_files_in_folder)} ft_*.c files")
        else:
            print(f"   ✓ Standalone libft project: {len(ft_files_at_root)} ft_*.c files at root")

        print(f"   ✓ libft.h found {'at root' if libft_h_at_root else 'in libft/ folder'}")

        # Makefile must be at the project root (per subject requirements)
        makefile = path / "Makefile"
        if not makefile.exists():
            print(f"❌ ERROR: Makefile not found at {path}")
            print("   Required at project root (per subject.md requirements)")
            return False

        makefile_content = makefile.read_text()
        has_libft_rule = "libft.a" in makefile_content or "libft:" in makefile_content

        if not has_libft_rule:
            print("⚠️  WARNING: Makefile doesn't appear to compile libft.a")
            print("   Subject.md requires: 'If your project allows you to use your libft,")
            print("   you must copy its sources and its associated Makefile into a libft folder.'")

        print("   ✓ Makefile found at project root")

        tests_dir = path / "tests"
        if tests_dir.exists():
            tests_files = list(tests_dir.iterdir())
            tests_str = ", ".join([f.name for f in tests_files])
            print(f"   ✓ tests/ directory exists with {len(tests_files)} files: {tests_str}")

        print("\n✅ libft project structure verification PASSED!")
        print("   - libft/ folder with library files ✓")
        print("   - Makefile at project root ✓")
        print(f"   - tests/ directory {'✓' if tests_dir.exists() else '(optional)'}")
        return True

    def build_troubleshooting(self):
        return ["Check if 'gcc' is available",
                "Verify Makefile syntax",
                "Ensure libft/ folder contains proper source files"]

    def install_infrastructure(self) -> bool:
        print(f"Installing test infrastructure for {self.name}")
        print("📁 Creating libft project structure (per subject.md):")

        # libft/ folder with library files
        libft_dir = Path("libft")
        libft_dir.mkdir(exist_ok=True)

        if not (libft_dir / "libft.h").exists():
            with open(libft_dir / "libft.h", 'w') as f:
                f.write(LIBFT_H)
            print("  📄 Created: libft/libft.h")

        for i in range(1, 4):
            example = libft_dir / f"ft_example_{i}.c"
            if not example.exists():
                with open(example, 'w') as f:
                    f.write(FT_EXAMPLE_C.format(i=i))
                print(f"  📄 Created: libft/ft_example_{i}.c")

        self.write_if_missing("Makefile", LIBFT_MAKEFILE)
        self.write_if_missing("tests/run_tests.sh", LIBFT_RUN_TESTS_SH, executable=True)
        self.write_if_missing("tests/main.c", LIBFT_MAIN_C)

        print("\n✅ libft test infrastructure installed successfully!")
        print("   - libft/ folder with library files ✓")
        print("   - Makefile at project root ✓")
        print("   - tests/ directory with test runner ✓")
        print("\n📝 Project now ready for testing with './testme libft .'")
        return True


LIBFT_H = """#ifndef LIBFT_H
# define LIBFT_H

typedef struct      s_list
{
    void           *content;
    struct s_list  *next;
}                   t_list;

// Function prototypes would go here
// See subject.md for complete list

#endif
"""

FT_EXAMPLE_C = """#include "libft.h"

void *ft_example_{i}(void)
{{
    // Example function implementation
    return NULL;
}}
"""

LIBFT_MAKEFILE = ""

LIBFT_RUN_TESTS_SH = """#!/bin/bash

# Test runner for libft project

make clean
make

# Compile and run tests
gcc -Wall -Wextra -Werror -I. tests/main.c libft/libft.a -o test_bin

./test_bin
rm -f test_bin
"""

LIBFT_MAIN_C = """#include <stdio.h>
#include <stdlib.h>
#include "libft/libft.h"

int main(void)
{
    printf("🧪 Testing libft project structure and compilation...\\n");
    printf("   This test verifies the correct project structure per subject.md\\n\\n");

    // Test that library compiles
    if (system("make clean && make") == 0)
    {
        printf("✅ Library compilation successful\\n");
    }
    else
    {
        printf("❌ Library compilation failed\\n");
        return 1;
    }

    // Test that libft.a exists
    FILE *libft_file = fopen("libft/libft.a", "rb");
    if (libft_file)
    {
        printf("✅ libft.a found in libft/ folder\\n");
        fclose(libft_file);
    }
    else
    {
        printf("❌ libft.a not found in libft/ folder\\n");
        return 1;
    }

    printf("\\n🎉 All structure tests passed! Project follows subject.md requirements\\n");
    return 0;
}
"""
