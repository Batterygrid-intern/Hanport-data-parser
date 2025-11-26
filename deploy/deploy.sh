#!/bin/bash
# Main deployment orchestrator

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo "==========================================="
echo "Hanport Service Deployment"
echo "==========================================="
echo ""

# Install dependencies
if [ -f "install-deps.sh" ]; then
    echo "[1/3] Installing dependencies..."
    bash install-deps.sh
    echo ""
fi

# Run main install
echo "[2/3] Installing Hanport service..."
bash install.sh
echo ""

# Start service
echo "[3/3] Starting service..."
sudo systemctl daemon-reload
sudo systemctl enable hanport
sudo systemctl start hanport

echo ""
echo "==========================================="
echo "✓ Deployment complete!"
echo "==========================================="
echo ""
echo "Service status:"
sudo systemctl status hanport --no-pager

echo ""
echo "Useful commands:"
echo "  sudo systemctl status hanport   - Check service status"
echo "  sudo systemctl restart hanport  - Restart service"
echo "  sudo journalctl -u hanport -f   - View live logs"
