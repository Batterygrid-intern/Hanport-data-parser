#!/bin/bash
# Build Hanport on Raspberry Pi (ARM64)

set -e

echo "==========================================="
echo "Hanport Build Script for Raspberry Pi"
echo "==========================================="
echo ""

# Check architecture
ARCH=$(uname -m)
if [[ "$ARCH" != "aarch64" && "$ARCH" != "armv7l" ]]; then
    echo "⚠️  Warning: This script is designed for ARM architecture"
    echo "   Detected: $ARCH"
    echo ""
fi

# Update package lists
echo "[1/4] Updating package lists..."
sudo apt-get update

# Install build dependencies
echo ""
echo "[2/4] Installing build dependencies..."
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libcrypto++-dev

# Check if libmodbus is installed
if ! ldconfig -p | grep -q libmodbus; then
    echo ""
    echo "Installing libmodbus from source..."
    cd /tmp
    git clone https://github.com/stephane/libmodbus.git
    cd libmodbus
    ./autogen.sh
    ./configure --prefix=/usr/local
    make
    sudo make install
    sudo ldconfig
    cd -
fi

# Check if Paho MQTT is installed
if ! ldconfig -p | grep -q libpaho-mqtt; then
    echo ""
    echo "Installing Paho MQTT C library..."
    cd /tmp
    git clone https://github.com/eclipse/paho.mqtt.c.git
    cd paho.mqtt.c
    cmake -Bbuild -H. -DPAHO_WITH_SSL=ON -DPAHO_BUILD_STATIC=OFF
    cmake --build build
    sudo cmake --build build --target install
    sudo ldconfig
    cd -
    
    echo ""
    echo "Installing Paho MQTT C++ library..."
    cd /tmp
    git clone https://github.com/eclipse/paho.mqtt.cpp.git
    cd paho.mqtt.cpp
    cmake -Bbuild -H. -DPAHO_BUILD_STATIC=OFF
    cmake --build build
    sudo cmake --build build --target install
    sudo ldconfig
    cd -
fi

# Check if spdlog is installed (needed at build time for headers)
if [ ! -d "/usr/local/include/spdlog" ] && [ ! -d "/usr/include/spdlog" ]; then
    echo ""
    echo "Installing spdlog (header-only library)..."
    cd /tmp
    git clone https://github.com/gabime/spdlog.git
    cd spdlog
    cmake -B build -DSPDLOG_BUILD_SHARED=OFF
    sudo cmake --build build --target install
    cd -
fi

# Build Hanport
echo ""
echo "[3/4] Building Hanport..."
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

# Clean previous build
rm -rf build bin

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

echo ""
echo "[4/4] Build complete!"
echo ""
ls -lh bin/

echo ""
echo "==========================================="
echo "✓ Build successful!"
echo "==========================================="
echo ""
echo "Binary: $(readlink -f bin/Hanport-data)"
echo "Architecture: $(file bin/Hanport-data | grep -oP '(x86-64|ARM aarch64|ARM)')"
echo ""
echo "Next steps:"
echo "  1. Test: ./bin/Hanport-data configs/app.ini"
echo "  2. Package: bash packbinaries/cpbinaries.sh"
echo "  3. Deploy: bash deploy/package.sh"
