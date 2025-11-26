#!/bin/bash
# Package source code for building on Raspberry Pi

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

VERSION=$(date +%Y%m%d_%H%M%S)
TARBALL="hanport-source-${VERSION}.tar.gz"

echo "==========================================="
echo "Hanport Source Packager"
echo "==========================================="
echo ""

echo "Creating source tarball: $TARBALL"

# Create tarball excluding build artifacts
tar czf "$TARBALL" \
    --exclude='build' \
    --exclude='bin' \
    --exclude='*.o' \
    --exclude='*.a' \
    --exclude='CMakeFiles' \
    --exclude='CMakeCache.txt' \
    --exclude='*.zip' \
    --exclude='*.tar.gz' \
    --exclude='.git' \
    --exclude='exploration-tests' \
    src/ \
    include/ \
    tests/ \
    configs/ \
    scripts/ \
    packbinaries/ \
    deploy/ \
    CMakeLists.txt \
    README.md

TARBALL_SIZE=$(du -h "$TARBALL" | cut -f1)

echo ""
echo "==========================================="
echo "✓ Source package created!"
echo "==========================================="
echo ""
echo "Package: $TARBALL"
echo "Size: $TARBALL_SIZE"
echo ""
echo "Transfer to Pi:"
echo "  scp $TARBALL pi@hanport:/tmp/"
echo ""
echo "Build on Pi:"
echo "  ssh pi@hanport"
echo "  cd /tmp"
echo "  tar xzf $TARBALL"
echo "  cd Hanport-data-parser"
echo "  bash scripts/build-on-pi.sh"
