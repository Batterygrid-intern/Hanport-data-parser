#!/bin/bash
# Debug Hanport service issues

echo "==========================================="
echo "Hanport Service Debugger"
echo "==========================================="
echo ""

echo "[1] Service Status:"
sudo systemctl status hanport --no-pager
echo ""

echo "[2] Recent Journal Logs:"
sudo journalctl -u hanport -n 50 --no-pager
echo ""

echo "[3] Binary Info:"
ls -la /usr/local/bin/Hanport-data
file /usr/local/bin/Hanport-data
echo ""

echo "[4] Library Dependencies:"
ldd /usr/local/bin/Hanport-data
echo ""

echo "[5] Config File:"
if [ -f /etc/hanport/config.ini ]; then
    ls -la /etc/hanport/config.ini
    echo "First 20 lines:"
    head -20 /etc/hanport/config.ini
else
    echo "Config file not found!"
fi
echo ""

echo "[6] Permissions:"
echo "User hanport exists: $(id hanport 2>&1)"
echo "Working dir: $(ls -ld /var/lib/hanport 2>&1)"
echo "Log dir: $(ls -ld /var/log/hanport 2>&1)"
echo ""

echo "[7] Manual Test (as hanport user):"
echo "Run this command to test manually:"
echo "sudo -u hanport LD_LIBRARY_PATH=/opt/hanport/lib:/usr/local/lib /usr/local/bin/Hanport-data /etc/hanport/config.ini"
