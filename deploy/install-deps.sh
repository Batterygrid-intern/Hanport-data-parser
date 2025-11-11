#!/usr/bin/env bash
set -euo pipefail

# Install runtime dependencies for hanport service
# Supports Debian/Ubuntu (apt), RHEL/CentOS/Fedora (dnf/yum), Arch (pacman)

echo "Installing runtime dependencies for hanport..."

# Detect distro
if [[ -f /etc/os-release ]]; then
  . /etc/os-release
  DISTRO=$ID
else
  echo "Warning: Cannot detect distro. Install libmodbus, libpaho-mqtt, libssl manually." >&2
  exit 1
fi

case "$DISTRO" in
  ubuntu|debian)
    echo "Detected Debian/Ubuntu — using apt"
    sudo apt-get update -qq
    sudo apt-get install -y \
      libmodbus5 \
      libssl3 \
      libstdc++6 \
      libc6
    # Note: libpaho-mqtt may not be in default repos; if not, install from source or PPA
    # If you built paho locally, the .so files are in /usr/local/lib (already in ld.so.conf or rpath)
    echo "Runtime dependencies installed (apt)."
    ;;
  fedora|rhel|centos|rocky|almalinux)
    echo "Detected RHEL/Fedora — using dnf/yum"
    if command -v dnf &>/dev/null; then
      PKG=dnf
    else
      PKG=yum
    fi
    sudo $PKG install -y \
      libmodbus \
      openssl-libs \
      libstdc++
    echo "Runtime dependencies installed ($PKG)."
    ;;
  arch|manjaro)
    echo "Detected Arch — using pacman"
    sudo pacman -Sy --noconfirm \
      libmodbus \
      openssl \
      gcc-libs
    echo "Runtime dependencies installed (pacman)."
    ;;
  *)
    echo "Unsupported distro: $DISTRO" >&2
    echo "Install these packages manually: libmodbus, libssl, libpaho-mqtt (C and C++)" >&2
    exit 1
    ;;
esac

# Check if paho MQTT C/C++ is installed in /usr/local (common when built from source)
if [[ ! -f /usr/local/lib/libpaho-mqttpp3.so ]] && [[ ! -f /usr/lib/libpaho-mqttpp3.so ]]; then
  echo ""
  echo "WARNING: libpaho-mqttpp3 not found in /usr/local/lib or /usr/lib."
  echo "If you built Paho MQTT C++ from source, ensure the .so files are installed and visible via ldconfig."
  echo "Run: sudo ldconfig"
  echo "Or bundle the .so files in the deployment package and set LD_LIBRARY_PATH in the systemd unit."
  echo ""
fi

echo "Dependency installation complete."
