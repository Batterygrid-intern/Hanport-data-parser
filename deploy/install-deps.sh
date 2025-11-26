#!/bin/bash
# Install runtime dependencies

set -e

echo "Installing Hanport runtime dependencies..."

if command -v apt-get &> /dev/null; then
    sudo apt-get update
    sudo apt-get install -y libssl3 libcrypto++8
elif command -v dnf &> /dev/null; then
    sudo dnf install -y openssl-libs cryptopp
elif command -v pacman &> /dev/null; then
    sudo pacman -S --noconfirm openssl crypto++
else
    echo "⚠️  Unknown package manager. Please install OpenSSL manually."
fi

echo "✓ Dependencies installed"
