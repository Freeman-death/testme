#!/bin/bash
# TestMe CLI Installer
# Usage: curl -sSL https://raw.githubusercontent.com/AyoubMzian-Dev/testme/main/install.sh | bash

set -e

# Detect OS
OS=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCH=$(uname -m)

echo "Installing TestMe CLI..."

# Create bin and data directories
mkdir -p "$HOME/.local/bin" "$HOME/.local/share/testme"

# Download the repository tarball (gets both the CLI and the per-project test suites)
TMP_DIR=$(mktemp -d)
curl -sSL "https://api.github.com/repos/AyoubMzian-Dev/testme/tarball/main" -o "$TMP_DIR/testme.tar.gz"
tar -xzf "$TMP_DIR/testme.tar.gz" -C "$TMP_DIR"

# Locate the extracted directory (named <owner>-<repo>-<ref>)
SRC_DIR=$(find "$TMP_DIR" -maxdepth 1 -type d -name "AyoubMzian-Dev-testme-*" | head -n1)
if [ -z "$SRC_DIR" ]; then
    echo "❌ Failed to download TestMe repository"
    rm -rf "$TMP_DIR"
    exit 1
fi

# Install the CLI
cp "$SRC_DIR/testme" "$HOME/.local/bin/testme"
chmod +x "$HOME/.local/bin/testme"

# Install per-project test suites (projects/ contains one file per project)
rm -rf "$HOME/.local/share/testme/projects"
cp -r "$SRC_DIR/projects" "$HOME/.local/share/testme/projects"

rm -rf "$TMP_DIR"

# Add to PATH if not already there
if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
    echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$HOME/.bashrc"
    echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$HOME/.zshrc" 2>/dev/null || true
    echo "Added ~/.local/bin to PATH. Please restart your shell or run: source ~/.bashrc"
fi

echo "✅ TestMe CLI installed successfully!"
echo "   Run: testme --help"
