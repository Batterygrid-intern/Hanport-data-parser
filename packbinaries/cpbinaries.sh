#!/bin/bash
# deploy.sh - Just pack the files

DEPLOY_DIR="../deploy"

# Clean and create structure
mkdir -p "$DEPLOY_DIR/bin"
mkdir -p "$DEPLOY_DIR/lib"

echo "Packing files..."

# Copy binary
cp ../bin/Hanport-data "$DEPLOY_DIR/bin/"
echo "✓ Binary copied"

# Copy libraries
cp -L /usr/local/lib/libpaho-mqttpp3.so.1 "$DEPLOY_DIR/lib/"
cp -L /usr/local/lib/libpaho-mqtt3as.so.1 "$DEPLOY_DIR/lib/"
cp -L /usr/local/lib/libmodbus.so.5 "$DEPLOY_DIR/lib/"
echo "✓ Libraries copied"

