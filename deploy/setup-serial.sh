#!/bin/bash

set -e

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Error: This script must be run with sudo"
    exit 1
fi

# Enable UART hardware in /boot/firmware/config.txt
echo "Configuring UART in /boot/firmware/config.txt..."
sed -i.bak '/^enable_uart=/d' /boot/firmware/config.txt
echo "enable_uart=1" | tee -a /boot/firmware/config.txt > /dev/null

# Disable Bluetooth overlay in /boot/firmware/config.txt
echo "Disabling Bluetooth in /boot/firmware/config.txt..."
sed -i '/^dtoverlay=disable-bt/d' /boot/firmware/config.txt
echo "dtoverlay=disable-bt" | tee -a /boot/firmware/config.txt > /dev/null

# Remove serial console from /boot/firmware/cmdline.txt and backup original
echo "Modifying /boot/firmware/cmdline.txt..."
cp /boot/firmware/cmdline.txt /boot/firmware/cmdline.txt.bak
sed -i.tmp 's/console=serial0,[0-9]\+ //g' /boot/firmware/cmdline.txt
sed -i.tmp 's/console=ttyAMA0,[0-9]\+ //g' /boot/firmware/cmdline.txt
rm -f /boot/firmware/cmdline.txt.tmp

# Disable serial-getty service on ttyAMA0
echo "Disabling serial-getty service..."
systemctl disable serial-getty@ttyAMA0.service 2>/dev/null || true
systemctl stop serial-getty@ttyAMA0.service 2>/dev/null || true

# Disable Bluetooth related services
echo "Disabling Bluetooth services..."
systemctl disable hciuart.service 2>/dev/null || true
systemctl disable bluetooth.service 2>/dev/null || true
systemctl stop hciuart.service 2>/dev/null || true
systemctl stop bluetooth.service 2>/dev/null || true

echo ""
echo "✓ UART enabled, serial console disabled on ttyAMA0, and Bluetooth disabled."
echo ""
echo "Rebooting in 5 seconds..."
sleep 5
reboot
