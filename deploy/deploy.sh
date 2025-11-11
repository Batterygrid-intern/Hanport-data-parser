#!/usr/bin/env bash
set -euo pipefail

# Automated deployment script for hanport (runs after extracting the package)
# Usage (after extracting tarball/zip):
#   cd hanport-package
#   sudo ./deploy.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "====================================="
echo "Hanport Service Deployment Installer"
echo "====================================="
echo ""

# Step 1: Install runtime dependencies
echo "[1/3] Installing runtime dependencies..."
if [[ -f "$SCRIPT_DIR/install-deps.sh" ]]; then
  bash "$SCRIPT_DIR/install-deps.sh"
else
  echo "Warning: install-deps.sh not found. Skipping dependency install." >&2
fi
echo ""

# Step 2: Run the main install script
echo "[2/3] Installing hanport binary, config and systemd unit..."
if [[ -f "$SCRIPT_DIR/install.sh" ]]; then
  bash "$SCRIPT_DIR/install.sh" --no-start
else
  echo "Error: install.sh not found in $SCRIPT_DIR" >&2
  exit 1
fi
echo ""

# Step 3: Start the service and show status
echo "[3/3] Starting hanport.service..."
sudo systemctl daemon-reload
sudo systemctl enable hanport.service
sudo systemctl restart hanport.service
sudo systemctl status hanport.service --no-pager -l || true
echo ""

echo "====================================="
echo "Deployment complete!"
echo "====================================="
echo ""
echo "Check service status:  sudo systemctl status hanport.service"
echo "View logs (journal):   sudo journalctl -u hanport.service -f"
echo "View logs (file):      sudo tail -F /var/log/hanport/hanport.log"
echo ""
