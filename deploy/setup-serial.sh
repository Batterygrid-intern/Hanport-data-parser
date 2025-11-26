#!/bin/bash

# Enable UART hardware in /boot/config.txt (add or replace enable_uart=1)
sudo sed -i '/^enable_uart=/d' /boot/config.txt
echo "enable_uart=1" | sudo tee -a /boot/config.txt

# Disable Bluetooth overlay in /boot/config.txt
sudo sed -i '/^dtoverlay=disable-bt/d' /boot/firmware/config.txt
echo "dtoverlay=disable-bt" | sudo tee -a /boot/firmware/config.txt

# Remove serial console from /boot/cmdline.txt and backup original
sudo cp /boot/firmware/cmdline.txt /boot/firmware/cmdline.txt.bak
sudo sed -i 's/ console=serial0,115200//g' /boot/firmware/cmdline.txt
sudo sed -i 's/ console=ttyAMA0,115200//g' /boot/firmware/cmdline.txt

# Disable serial-getty service on ttyAMA0
sudo systemctl disable serial-getty@ttyAMA0.service
sudo systemctl stop serial-getty@ttyAMA0.service

# Disable Bluetooth related services
sudo systemctl disable hciuart.service
sudo systemctl disable bluetooth.service
sudo systemctl stop hciuart.service
sudo systemctl stop bluetooth.service

echo "UART enabled, serial console disabled on ttyAMA0, and Bluetooth disabled."
echo "Rebooting in 5 seconds..."
sleep 5
sudo reboot
