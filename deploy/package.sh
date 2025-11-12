#!/bin/bash
# Package deployment files into a .zip archive

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR/.."

VERSION=$(date +%Y%m%d_%H%M%S)
PACKAGE_NAME="hanport-deploy-${VERSION}.zip"

echo "==========================================="
echo "Hanport Deployment Packager"
echo "==========================================="
echo ""

# Check that required files exist
if [ ! -f "deploy/bin/Hanport-data" ]; then
    echo "❌ Error: Binary not found in deploy/bin/"
    echo "   Run: bash packbinaries/cpbinaries.sh first"
    exit 1
fi

if [ ! -d "deploy/lib" ] || [ -z "$(ls -A deploy/lib)" ]; then
    echo "❌ Error: No libraries found in deploy/lib/"
    echo "   Run: bash packbinaries/cpbinaries.sh first"
    exit 1
fi

echo "📦 Creating package: ${PACKAGE_NAME}"
echo ""

# Create temporary staging directory
STAGING=$(mktemp -d)
trap "rm -rf $STAGING" EXIT

# Copy files to staging
mkdir -p "$STAGING/hanport"
cp -r deploy/bin "$STAGING/hanport/"
cp -r deploy/lib "$STAGING/hanport/"
cp deploy/*.ini "$STAGING/hanport/" 2>/dev/null || true
cp deploy/hanport.service "$STAGING/hanport/" 2>/dev/null || cp deploy/hanport.serivce "$STAGING/hanport/hanport.service" 2>/dev/null || true
cp deploy/install.sh "$STAGING/hanport/"

# Create install scripts if they don't exist
if [ ! -f "deploy/install-deps.sh" ]; then
    cat > "$STAGING/hanport/install-deps.sh" << 'EOF'
#!/bin/bash
# Install runtime dependencies

set -e

echo "Installing Hanport runtime dependencies..."

if command -v apt-get &> /dev/null; then
    sudo apt-get update
    sudo apt-get install -y libssl3 libcrypto++8
elif command -v dnf &> /dev/null; then
    sudo dnf install -y openssl-libs cryptopp
elif command -v pacman &> /dev/null; then
    sudo pacman -S --noconfirm openssl crypto++
else
    echo "⚠️  Unknown package manager. Please install OpenSSL manually."
fi

echo "✓ Dependencies installed"
EOF
    chmod +x "$STAGING/hanport/install-deps.sh"
fi

# Create deploy script if it doesn't exist
if [ ! -f "deploy/deploy.sh" ]; then
    cat > "$STAGING/hanport/deploy.sh" << 'EOF'
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
EOF
    chmod +x "$STAGING/hanport/deploy.sh"
fi

# Create the zip package
cd "$STAGING"
zip -r "$PACKAGE_NAME" hanport/ > /dev/null

# Move to project root
mv "$PACKAGE_NAME" "$SCRIPT_DIR/../$PACKAGE_NAME"

echo "✓ Package created: $PACKAGE_NAME"
echo ""
echo "Package contents:"
unzip -l "$SCRIPT_DIR/../$PACKAGE_NAME" | grep -v "^Archive:" | head -20

PACKAGE_SIZE=$(du -h "$SCRIPT_DIR/../$PACKAGE_NAME" | cut -f1)
echo ""
echo "==========================================="
echo "Package: $PACKAGE_NAME"
echo "Size: $PACKAGE_SIZE"
echo "==========================================="
echo ""
echo "Next steps:"
echo "  1. Upload: scp $PACKAGE_NAME user@target:/tmp/"
echo "  2. On target: unzip $PACKAGE_NAME && cd hanport && sudo bash deploy.sh"
echo "  3. Or use FTP: bash ftpscripts/uploadzip.sh $PACKAGE_NAME"
