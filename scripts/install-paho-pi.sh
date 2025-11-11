#!/usr/bin/env bash
set -euo pipefail

# Install Paho MQTT C and C++ libraries on Raspberry Pi (or any ARM/Linux system)
# This is a one-time setup required before building the Hanport project
# Usage: bash scripts/install-paho-pi.sh

echo "========================================="
echo "Paho MQTT C/C++ Installation Script"
echo "========================================="
echo ""
echo "This will build and install:"
echo "  - Paho MQTT C (v1.3.13)"
echo "  - Paho MQTT C++ (v1.4.1)"
echo ""
echo "Installation location: /usr/local/lib"
echo "Estimated time: ~10 minutes on Raspberry Pi 4"
echo ""
read -p "Continue? [y/N] " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Installation cancelled."
    exit 0
fi

# Install build dependencies
echo ""
echo "[1/5] Installing build dependencies..."
sudo apt update
sudo apt install -y build-essential cmake git libssl-dev

# Create build directory
BUILD_DIR="${HOME}/paho-build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# ============================================
# Part 1: Build and install Paho MQTT C
# ============================================
echo ""
echo "[2/5] Cloning Paho MQTT C..."
if [[ -d paho.mqtt.c ]]; then
    echo "Directory paho.mqtt.c already exists, removing..."
    rm -rf paho.mqtt.c
fi
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
git checkout v1.3.13

echo ""
echo "[3/5] Building Paho MQTT C..."
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DPAHO_WITH_SSL=ON \
  -DPAHO_BUILD_STATIC=OFF \
  -DPAHO_BUILD_SHARED=ON \
  -DPAHO_ENABLE_TESTING=OFF

cmake --build build -j$(nproc)

echo "Installing Paho MQTT C..."
sudo cmake --install build

echo "✓ Paho MQTT C installed"

# ============================================
# Part 2: Build and install Paho MQTT C++
# ============================================
echo ""
echo "[4/5] Cloning Paho MQTT C++..."
cd "$BUILD_DIR"
if [[ -d paho.mqtt.cpp ]]; then
    echo "Directory paho.mqtt.cpp already exists, removing..."
    rm -rf paho.mqtt.cpp
fi
git clone https://github.com/eclipse/paho.mqtt.cpp.git
cd paho.mqtt.cpp
git checkout v1.4.1

echo ""
echo "Building Paho MQTT C++..."
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DPAHO_BUILD_STATIC=OFF \
  -DPAHO_BUILD_SHARED=ON \
  -DPAHO_WITH_SSL=ON

cmake --build build -j$(nproc)

echo "Installing Paho MQTT C++..."
sudo cmake --install build

echo "✓ Paho MQTT C++ installed"

# ============================================
# Part 3: Update library cache
# ============================================
echo ""
echo "[5/5] Updating library cache..."
sudo ldconfig

# Verify installation
echo ""
echo "========================================="
echo "Installation Complete!"
echo "========================================="
echo ""
echo "Installed libraries:"
ls -lh /usr/local/lib/libpaho-mqtt* 2>/dev/null || echo "Warning: Libraries not found in /usr/local/lib"

echo ""
echo "Verifying pkg-config:"
if pkg-config --exists paho-mqtt3c 2>/dev/null; then
    echo "  ✓ paho-mqtt3c: $(pkg-config --modversion paho-mqtt3c)"
else
    echo "  ⚠ paho-mqtt3c not found in pkg-config"
fi

if pkg-config --exists paho-mqttpp3 2>/dev/null; then
    echo "  ✓ paho-mqttpp3: $(pkg-config --modversion paho-mqttpp3)"
else
    echo "  ⚠ paho-mqttpp3 not found in pkg-config"
fi

echo ""
echo "You can now build the Hanport project:"
echo "  cd ~/Hanport-data-parser"
echo "  cmake -B build"
echo "  cmake --build build -j\$(nproc)"
echo ""
echo "Cleanup build directory (optional):"
echo "  rm -rf $BUILD_DIR"
echo ""
