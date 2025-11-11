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
