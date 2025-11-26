#!/bin/bash
# Uninstall Hanport service and cleanup

set -e

echo "==========================================="
echo "Hanport Service Uninstaller"
echo "==========================================="
echo ""

# Stop and disable service
echo "[1/6] Stopping service..."
if systemctl is-active --quiet hanport; then
    sudo systemctl stop hanport
    echo "  ✓ Service stopped"
else
    echo "  - Service not running"
fi

if systemctl is-enabled --quiet hanport 2>/dev/null; then
    sudo systemctl disable hanport
    echo "  ✓ Service disabled"
fi

# Remove systemd unit
echo ""
echo "[2/6] Removing systemd unit..."
if [ -f /etc/systemd/system/hanport.service ]; then
    sudo rm /etc/systemd/system/hanport.service
    sudo systemctl daemon-reload
    echo "  ✓ Unit file removed"
else
    echo "  - Unit file not found"
fi

# Remove binary
echo ""
echo "[3/6] Removing binary..."
if [ -f /usr/local/bin/Hanport-data ]; then
    sudo rm /usr/local/bin/Hanport-data
    echo "  ✓ Binary removed"
else
    echo "  - Binary not found"
fi

# Remove libraries
echo ""
echo "[4/6] Removing libraries..."
if [ -d /opt/hanport ]; then
    sudo rm -rf /opt/hanport
    echo "  ✓ Libraries removed"
else
    echo "  - Libraries not found"
fi

# Remove config
echo ""
echo "[5/6] Removing configuration..."
if [ -d /etc/hanport ]; then
    sudo rm -rf /etc/hanport
    echo "  ✓ Configuration removed"
else
    echo "  - Configuration not found"
fi

# Remove working directories (optional - preserves data)
echo ""
echo "[6/6] Cleaning up directories..."
read -p "Remove log files in /var/log/hanport? [y/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if [ -d /var/log/hanport ]; then
        sudo rm -rf /var/log/hanport
        echo "  ✓ Logs removed"
    fi
else
    echo "  - Logs kept"
fi

read -p "Remove working directory /var/lib/hanport? [y/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if [ -d /var/lib/hanport ]; then
        sudo rm -rf /var/lib/hanport
        echo "  ✓ Working directory removed"
    fi
else
    echo "  - Working directory kept"
fi

# Remove user (optional)
echo ""
read -p "Remove hanport user? [y/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if id -u hanport > /dev/null 2>&1; then
        sudo userdel hanport
        echo "  ✓ User removed"
    fi
else
    echo "  - User kept"
fi

echo ""
echo "==========================================="
echo "✓ Uninstall complete!"
echo "==========================================="
echo ""
echo "Note: System dependencies (libmodbus, paho-mqtt, openssl)"
echo "      were not removed as they may be used by other programs."
echo ""
echo "To remove them manually (Ubuntu/Debian):"
echo "  sudo apt-get remove --purge libmodbus5 libpaho-mqtt1.3 libpaho-mqttpp3"
