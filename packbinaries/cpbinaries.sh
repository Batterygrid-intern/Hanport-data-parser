#!/usr/bin/env bash
set -euo pipefail

# Copy Hanport-data binary and all its shared library dependencies
# This creates a lib/ directory with all required .so files for deployment

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DEPLOY_DIR="$REPO_ROOT/deploy"
BINARY_PATH="$REPO_ROOT/bin/Hanport-data"

echo "========================================="
echo "Hanport Binary and Library Packager"
echo "========================================="
echo ""

# Verify binary exists
if [[ ! -f "$BINARY_PATH" ]]; then
  echo "Error: Binary not found at $BINARY_PATH" >&2
  echo "Build the project first: cmake --build build -j\$(nproc)" >&2
  exit 1
fi

# Clean and create structure
rm -rf "$DEPLOY_DIR/bin" "$DEPLOY_DIR/lib"
mkdir -p "$DEPLOY_DIR/bin"
mkdir -p "$DEPLOY_DIR/lib"

echo "Binary: $BINARY_PATH"
echo "Deploy: $DEPLOY_DIR"
echo ""

# Copy binary
echo "[1/2] Copying binary..."
cp "$BINARY_PATH" "$DEPLOY_DIR/bin/"
chmod +x "$DEPLOY_DIR/bin/Hanport-data"
echo "✓ Binary copied: Hanport-data"

# Get list of shared libraries and copy them
echo ""
echo "[2/2] Copying shared library dependencies..."

# Extract library paths from ldd output (only from /usr/local/lib)
ldd "$BINARY_PATH" | grep '/usr/local/lib' | awk '{print $3}' | while read -r lib; do
  if [[ -f "$lib" ]]; then
    # Get the directory and base name
    lib_dir=$(dirname "$lib")
    lib_base=$(basename "$lib" | sed 's/\.so\..*//')
    
    # Copy all related files (actual .so files and symlinks)
    find "$lib_dir" -name "${lib_base}.so*" \( -type f -o -type l \) | while read -r sofile; do
      cp -P "$sofile" "$DEPLOY_DIR/lib/"
    done
    
    echo "  ✓ $(basename "$lib")"
  fi
done

# Verify what was copied
echo ""
echo "========================================="
echo "Package Summary"
echo "========================================="
echo "Binary:"
ls -lh "$DEPLOY_DIR/bin/"
echo ""
echo "Libraries:"
ls -lh "$DEPLOY_DIR/lib/" 2>/dev/null || echo "  (none)"
echo ""
echo "Total size: $(du -sh "$DEPLOY_DIR" | cut -f1)"
echo ""
echo "✓ Files ready in: $DEPLOY_DIR"
echo ""
echo "Next steps:"
echo "  1. Run: bash deploy/package.sh"
echo "  2. Deploy: scp hanport-deploy.tar.gz user@target:/tmp/"
echo ""
