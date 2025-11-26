# rpi-hanport
This is a project built on the raspberry pi 4 to read data from a swedish electricity meter using the hanport interface and to
transmit the data collected using modbus and mqtt protocol.

## Quick Installation Guide

### Prerequisites
- Raspberry Pi 4 (ARM64/aarch64) running Raspberry Pi OS or Ubuntu
- Internet connection for downloading dependencies
- SSH access to the Pi

### Installation Steps

#### 1. On Your Development Machine (x86_64)

Package the source code for transfer:
```bash
cd Hanport-data-parser
bash scripts/package-source.sh
```

Transfer to Raspberry Pi:
```bash
scp hanport-source-*.tar.gz pi@your-pi-hostname:/tmp/
```

#### 2. On Raspberry Pi

Extract and build the project:
```bash
cd /tmp
tar xzf hanport-source-*.tar.gz
cd Hanport-data-parser

# This script will:
# - Install build dependencies (cmake, g++, etc.)
# - Build and install libmodbus
# - Build and install Paho MQTT C and C++
# - Compile the Hanport project
bash scripts/build-on-pi.sh
```

Create deployment package:
```bash
# Copy binary and libraries to deployment folder
bash packbinaries/cpbinaries.sh

# Create deployment zip
bash deploy/package.sh
```

Install as system service:
```bash
cd /tmp
unzip hanport-deploy-*.zip
cd hanport

# This will:
# - Install runtime dependencies
# - Copy binary to /usr/local/bin/
# - Copy libraries to /opt/hanport/lib/
# - Create hanport system user
# - Install config to /etc/hanport/
# - Create log directory
# - Install and start systemd service
sudo bash deploy.sh
```

#### 3. Verify Installation

Check service status:
```bash
sudo systemctl status hanport
```

View logs:
```bash
# Application logs
sudo tail -f /var/log/hanport/hanport.log

# System logs
sudo journalctl -u hanport -f
```

#### 4. Configuration

Edit the configuration file:
```bash
sudo nano /etc/hanport/config.ini
```

After making changes, restart the service:
```bash
sudo systemctl restart hanport
```

### Uninstallation

To remove the service:
```bash
cd /tmp/hanport
sudo bash uninstall.sh
```

### Troubleshooting

**Serial port access issues:**
```bash
# Add hanport user to dialout group
sudo usermod -a -G dialout hanport
sudo systemctl restart hanport
```

**Check if service is running:**
```bash
sudo systemctl status hanport
```

**View recent errors:**
```bash
sudo journalctl -u hanport -n 50
```

---

# Developer Documentation

# Activate/Configure (UART) serial on the rpi4 (RX pin)

# Build Dependencies
* automake
* autoconf
* libtool
* openssl
* build-essential
* libssl-dev
  
## build tools 
* cmake version 3.16
* make
* build-essential

## Compilers
* Fully compliant c++17 compiler.
* C v11 compiler

## Install external libraries
### Libmodbus
* [Libmodbus](https://github.com/stephane/libmodbus) follow the installation instructions.

### PahoMqttCpp
* [PahoMqttCpp](https://github.com/eclipse-paho/paho.mqtt.cpp) follow the installation instructions for version 1.5

# Config
The program reads configurations from an ini file
Configurations are used for modbus and mqtt configruations,to set which serialport to read from and where to write log output too.
