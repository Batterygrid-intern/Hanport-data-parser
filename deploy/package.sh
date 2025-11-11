#!/usr/bin/env bash
set -euo pipefail

# Package hanport for deployment
# Creates a tarball with binary, config, systemd unit and install scripts
# Usage: ./package.sh [output-dir]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="${1:-$REPO_ROOT}"

PACKAGE_NAME="hanport-deploy"
PACKAGE_DIR="$OUTPUT_DIR/$PACKAGE_NAME"
TARBALL="$OUTPUT_DIR/$PACKAGE_NAME.tar.gz"

echo "Building deployment package..."

# Clean old package
rm -rf "$PACKAGE_DIR" "$TARBALL"

# Create package directory structure
mkdir -p "$PACKAGE_DIR"

# Copy binary
if [[ -f "$REPO_ROOT/bin/test" ]]; then
  cp "$REPO_ROOT/bin/test" "$PACKAGE_DIR/hanport"
  chmod 0755 "$PACKAGE_DIR/hanport"
  echo "✓ Binary copied: hanport"
else
  echo "Error: Binary not found at $REPO_ROOT/bin/test" >&2
  echo "Build the project first: cmake --build build -j\$(nproc)" >&2
  exit 1
fi

# Copy config
if [[ -f "$REPO_ROOT/configs/app.ini" ]]; then
  cp "$REPO_ROOT/configs/app.ini" "$PACKAGE_DIR/config.ini"
  echo "✓ Config copied: config.ini"
else
  echo "Warning: Config not found at $REPO_ROOT/configs/app.ini" >&2
fi

# Copy systemd unit
if [[ -f "$REPO_ROOT/deploy/hanport.service" ]]; then
  cp "$REPO_ROOT/deploy/hanport.service" "$PACKAGE_DIR/hanport.service"
  echo "✓ Systemd unit copied: hanport.service"
fi

# Copy install scripts
for script in install.sh install-deps.sh deploy.sh; do
  if [[ -f "$REPO_ROOT/deploy/$script" ]]; then
    cp "$REPO_ROOT/deploy/$script" "$PACKAGE_DIR/$script"
    chmod +x "$PACKAGE_DIR/$script"
    echo "✓ Script copied: $script"
  fi
done

# Bundle shared libraries for fully portable package
echo "Bundling shared libraries..."
mkdir -p "$PACKAGE_DIR/lib"

# Bundle Paho MQTT libraries
for lib in libpaho-mqttpp3.so* libpaho-mqtt3as.so* libpaho-mqtt3a.so* libpaho-mqtt3c.so* libpaho-mqtt3cs.so*; do
  if [[ -f "/usr/local/lib/$lib" ]]; then
    cp -P "/usr/local/lib/$lib" "$PACKAGE_DIR/lib/"
  fi
done

# Bundle libmodbus if installed in /usr/local
for lib in libmodbus.so*; do
  if [[ -f "/usr/local/lib/$lib" ]]; then
    cp -P "/usr/local/lib/$lib" "$PACKAGE_DIR/lib/"
  fi
done

if [[ -n "$(ls -A "$PACKAGE_DIR/lib" 2>/dev/null)" ]]; then
  echo "✓ Bundled shared libraries in lib/"
  ls -lh "$PACKAGE_DIR/lib/"
else
  echo "Warning: No libraries found in /usr/local/lib to bundle" >&2
  echo "The package will require runtime dependencies to be installed on target" >&2
fi

# Create README
cat > "$PACKAGE_DIR/README.txt" <<EOF
Hanport Service Deployment Package
===================================

This package contains the hanport binary, configuration, systemd unit,
and automated install scripts.

Prerequisites
-------------
- A systemd-based Linux distribution (Debian/Ubuntu, RHEL/Fedora, Arch, etc.)
- Root/sudo access on the target machine

Quick Install
-------------
1. Extract this package on the target machine:
   tar -xzf hanport-deploy.tar.gz
   cd hanport-deploy

2. Run the automated deployment script:
   sudo ./deploy.sh

   This will:
   - Install runtime dependencies (libmodbus, libssl, etc.)
   - Copy the binary to /usr/local/bin/hanport
   - Install config to /etc/hanport/config.ini
   - Install systemd unit and enable/start the service

Manual Install (if you prefer step-by-step)
--------------------------------------------
1. Install dependencies:
   sudo ./install-deps.sh

2. Install the service:
   sudo ./install.sh

3. Start the service:
   sudo systemctl start hanport.service

Checking Status
---------------
sudo systemctl status hanport.service
sudo journalctl -u hanport.service -f
sudo tail -F /var/log/hanport/hanport.log

Uninstall
---------
sudo systemctl stop hanport.service
sudo systemctl disable hanport.service
sudo rm /etc/systemd/system/hanport.service
sudo rm /usr/local/bin/hanport
sudo rm -rf /etc/hanport /var/log/hanport /var/lib/hanport
sudo systemctl daemon-reload

For more information, visit:
https://github.com/Batterygrid-intern/Hanport-data-parser
EOF

echo "✓ README.txt created"

# Create tarball
cd "$OUTPUT_DIR"
tar -czf "$TARBALL" "$PACKAGE_NAME"
echo ""
echo "✓ Package created: $TARBALL"
echo ""
echo "Deploy on target machine:"
echo "  scp $TARBALL user@target:/tmp/"
echo "  ssh user@target 'cd /tmp && tar -xzf $(basename "$TARBALL") && cd $PACKAGE_NAME && sudo ./deploy.sh'"
echo ""
