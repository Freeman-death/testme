#!/bin/bash
# TestMe CLI Installer
# Usage: curl -sSL https://raw.githubusercontent.com/AyoubMzian-Dev/testme/main/install.sh | bash

set -e

# Detect OS
OS=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCH=$(uname -m)

echo "Installing TestMe CLI..."

# Create bin directory
mkdir -p "$HOME/.local/bin"

# Download testme script (use API to avoid caching issues)
curl -sSL "https://api.github.com/repos/AyoubMzian-Dev/testme/contents/testme?ref=main" | python3 -c "import sys,json,base64; d=json.load(sys.stdin); open('$HOME/.local/bin/testme','wb').write(base64.b64decode(d['content']))"
chmod +x "$HOME/.local/bin/testme"

# Add to PATH if not already there
if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
    echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$HOME/.bashrc"
    echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$HOME/.zshrc" 2>/dev/null || true
    echo "Added ~/.local/bin to PATH. Please restart your shell or run: source ~/.bashrc"
fi

echo "✅ TestMe CLI installed successfully!"
echo "   Run: testme --help"
