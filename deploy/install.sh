#!/bin/bash
# Install Hanport service

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "Installing Hanport service..."

# 1. Copy binary to /usr/local/bin
echo "  → Installing binary..."
sudo cp "$SCRIPT_DIR/bin/Hanport-data" /usr/local/bin/
sudo chmod 755 /usr/local/bin/Hanport-data

# Grant capability to bind to privileged ports (< 1024)
echo "  → Granting network capabilities..."
sudo setcap 'cap_net_bind_service=+ep' /usr/local/bin/Hanport-data

# 2. Copy libraries to /opt/hanport/lib
echo "  → Installing libraries..."
sudo mkdir -p /opt/hanport/lib
sudo cp -P "$SCRIPT_DIR/lib/"* /opt/hanport/lib/
sudo ldconfig /opt/hanport/lib

# 3. Create hanport user if it doesn't exist
if ! id -u hanport > /dev/null 2>&1; then
    echo "  → Creating hanport user..."
    sudo useradd --system --no-create-home --shell /usr/sbin/nologin hanport
fi

# 4. Copy config to /etc/hanport
echo "  → Installing configuration..."
sudo mkdir -p /etc/hanport
if [ -f "$SCRIPT_DIR/bgs-lokal.ini" ]; then
    sudo cp "$SCRIPT_DIR/bgs-lokal.ini" /etc/hanport/config.ini
elif [ -f "$SCRIPT_DIR/config.ini" ]; then
    sudo cp "$SCRIPT_DIR/config.ini" /etc/hanport/config.ini
fi
sudo chown hanport:hanport /etc/hanport/config.ini
sudo chmod 640 /etc/hanport/config.ini

# 5. Create working directory
echo "  → Creating working directory..."
sudo mkdir -p /var/lib/hanport
sudo chown hanport:hanport /var/lib/hanport

# 6. Create log directory
echo "  → Creating log directory..."
sudo mkdir -p /var/log/hanport
sudo chown hanport:hanport /var/log/hanport

# 7. Install systemd unit
echo "  → Installing systemd unit..."
SERVICE_FILE="$SCRIPT_DIR/hanport.service"
if [ ! -f "$SERVICE_FILE" ]; then
    SERVICE_FILE="$SCRIPT_DIR/hanport.serivce"  # Handle typo
fi

if [ -f "$SERVICE_FILE" ]; then
    sudo cp "$SERVICE_FILE" /etc/systemd/system/hanport.service
else
    # Create default service file
    sudo tee /etc/systemd/system/hanport.service > /dev/null << 'EOF'
[Unit]
Description=Hanport Data Parser Service
After=network.target

[Service]
Type=simple
User=hanport
Group=hanport
WorkingDirectory=/var/lib/hanport
Environment="LD_LIBRARY_PATH=/opt/hanport/lib:/usr/local/lib"
ExecStart=/usr/local/bin/Hanport-data --config /etc/hanport/config.ini
Restart=always
RestartSec=10

# Security hardening
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=/var/log/hanport /var/lib/hanport

[Install]
WantedBy=multi-user.target
EOF
fi

echo "✓ Installation complete!"
echo ""
echo "Run the following to start the service:"
echo "  sudo systemctl daemon-reload"
echo "  sudo systemctl enable hanport"
echo "  sudo systemctl start hanport" 

