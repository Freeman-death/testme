"""ProjectTest base class shared by all per-project test suites.

Each supported student project gets its own module in this directory defining
a ProjectTest subclass with project-specific metadata, structure verification
and install templates. The generic build/test flow lives here so subclasses
only describe what differs between projects.
"""

import os
import shutil
import subprocess
from pathlib import Path


class ProjectTest:
    """Base class for a single project's test suite."""

    name = "base"
    description = "base project test suite"

    # Tools required to build and run this project's tests
    requirements = ["gcc", "make"]

    # Tools that only enable extra checks when present
    optional_tools = ["valgrind"]

    # Location of the project's own test runner (relative to project root)
    test_file = "tests/run_tests.sh"
    build_cmd = "make"
    test_cmd = "./tests/run_tests.sh normal"

    # ------------------------------------------------------------------ #
    # Hooks subclasses may override
    # ------------------------------------------------------------------ #

    def verify_structure(self, path: Path) -> bool:
        """Verify project structure against the subject. Return False on fatal errors."""
        return True

    def post_build_check(self, path: Path) -> bool:
        """Verify build artifacts after `make` succeeded. Return False on fatal errors."""
        return True

    def build_troubleshooting(self):
        """Extra tips printed when the build fails."""
        return []

    def install_infrastructure(self) -> bool:
        """Create test infrastructure in the current working directory."""
        print(f"No install template defined for {self.name}")
        return False

    # ------------------------------------------------------------------ #
    # Shared helpers
    # ------------------------------------------------------------------ #

    def check_requirements(self) -> bool:
        missing = [tool for tool in self.requirements if shutil.which(tool) is None]
        if missing:
            print(f"Missing required tools: {', '.join(missing)}")
            return False

        optional_missing = [t for t in self.optional_tools if shutil.which(t) is None]
        if optional_missing:
            print(f"⚠️  Optional tools not found (some tests may be skipped): "
                  f"{', '.join(optional_missing)}")
        return True

    def run_make(self, *args) -> subprocess.CompletedProcess:
        return subprocess.run(["make", *args], capture_output=True, text=True)

    def write_if_missing(self, rel_path: str, content: str,
                         executable: bool = False) -> None:
        """Write a template file, never overwriting something the user already has."""
        target = Path(rel_path)
        if target.exists():
            print(f"  ⏭️  Skipping (already exists): {rel_path}")
            return
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(content)
        if executable:
            target.chmod(0o755)
        print(f"  📄 Created: {rel_path}")

    # ------------------------------------------------------------------ #
    # Generic test flow
    # ------------------------------------------------------------------ #

    def run_tests(self, path) -> bool:
        path = Path(path).resolve()
        if not path.exists():
            print(f"Error: Path does not exist: {path}")
            return False

        # For standalone projects without their own test runner - that's OK
        project_file = path / self.test_file
        if not project_file.exists():
            print(f"⚠️  Note: {self.test_file} not found - will use default test runner")
            project_file = None

        if not self.verify_structure(path):
            return False

        print(f"\nTesting {self.name} at {path}")
        if project_file:
            print(f"Using: {project_file}")

        if not self.check_requirements():
            return False

        original_cwd = os.getcwd()
        os.chdir(path)

        try:
            print(f"\n🔧 Building with: {self.build_cmd}")
            result = self.run_make()
            if result.returncode != 0:
                print("❌ Build failed:")
                print(result.stdout)
                print(result.stderr)
                print("\n💡 Troubleshooting:")
                for tip in self.build_troubleshooting():
                    print(f"   - {tip}")
                return False
            print("✅ Build successful")

            if not self.post_build_check(path):
                return False

            if project_file is None:
                return self.run_basic_compilation_check()
            return self.run_test_script()
        finally:
            os.chdir(original_cwd)

    def run_basic_compilation_check(self) -> bool:
        """Fallback when the project has no test runner: verify a clean rebuild."""
        print("\n🧪 Running basic compilation test...")
        result = self.run_make("clean")
        if result.returncode != 0:
            print("❌ Clean failed:")
            print(result.stdout)
            print(result.stderr)
            return False
        result = self.run_make()
        if result.returncode == 0:
            print("✅ Library compiles successfully")
            self.run_make("fclean")
            print(f"\n🎉 {self.name} structure and compilation PASSED")
            return True
        print("❌ Compilation failed:")
        print(result.stdout)
        print(result.stderr)
        return False

    def run_test_script(self) -> bool:
        print(f"\n🧪 Running tests with: {self.test_cmd}")
        result = subprocess.run(self.test_cmd.split(), capture_output=True, text=True)
        self.run_make("fclean")
        print(result.stdout)
        if result.stderr:
            print("STDERR:")
            print(result.stderr)
        if result.returncode == 0:
            print(f"\n🎉 {self.name} tests PASSED")
            return True
        print(f"\n💥 {self.name} tests FAILED")
        print("💡 Check test output above for specific failures")
        return False
