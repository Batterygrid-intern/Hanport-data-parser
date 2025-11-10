#!/usr/bin/env bash
set -euo pipefail

# Install script for Hanport service
# Usage: sudo ./install.sh [--no-start]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_BIN="$SCRIPT_DIR/../bin/test"
TARGET_BIN="/usr/local/bin/hanport"
CONFIG_SRC="$SCRIPT_DIR/../configs/app.ini"
CONFIG_DST="/etc/hanport/config.ini"
SERVICE_SRC="$SCRIPT_DIR/hanport.service"
SERVICE_DST="/etc/systemd/system/hanport.service"

NO_START=0
if [[ ${1-} == "--no-start" ]]; then
  NO_START=1
fi

if [[ ! -f "$BUILD_BIN" ]]; then
  echo "Error: built binary not found at $BUILD_BIN"
  echo "Build the project first (cmake --build build) and re-run this script." >&2
  exit 2
fi

echo "Installing hanport service..."

sudo useradd --system --no-create-home --shell /usr/sbin/nologin hanport || true

sudo mkdir -p /etc/hanport
sudo mkdir -p /var/log/hanport
sudo mkdir -p /var/lib/hanport

echo "Installing binary to $TARGET_BIN"
sudo install -m 0755 "$BUILD_BIN" "$TARGET_BIN"
sudo chown root:hanport "$TARGET_BIN"

if [[ -f "$CONFIG_SRC" ]]; then
  echo "Installing config to $CONFIG_DST"
  sudo install -m 0640 "$CONFIG_SRC" "$CONFIG_DST"
  sudo chown hanport:hanport "$CONFIG_DST"
else
  echo "Warning: config file $CONFIG_SRC not found — create $CONFIG_DST manually or pass a different --config when starting the service" >&2
fi

echo "Setting up logs dir and file"
sudo touch /var/log/hanport/hanport.log || true
sudo chown -R hanport:hanport /var/log/hanport
sudo chmod 0750 /var/log/hanport

echo "Preparing working directory"
sudo chown -R hanport:hanport /var/lib/hanport
sudo chmod 0750 /var/lib/hanport

echo "Installing systemd unit to $SERVICE_DST"
sudo install -m 0644 "$SERVICE_SRC" "$SERVICE_DST"

echo "Reloading systemd daemon"
sudo systemctl daemon-reload

echo "Enabling hanport.service"
sudo systemctl enable hanport.service

if [[ $NO_START -eq 0 ]]; then
  echo "Starting hanport.service"
  sudo systemctl restart hanport.service
  sudo systemctl status hanport.service --no-pager
else
  echo "Installation complete. Service enabled but not started (--no-start)."
fi

echo "Done. Check logs with: sudo journalctl -u hanport.service -f"
