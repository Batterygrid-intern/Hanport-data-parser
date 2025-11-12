#!/usr/bin/env bash
set -euo pipefail

# FTP Zip File Fetcher and Installer
# Downloads deployment package (.zip) from FTP, extracts it, runs install.sh, and cleans up
# Usage: ./fetchzip.sh [--config ftp.conf]

# Default configuration
FTP_HOST="${FTP_HOST:-ftp.example.com}"
FTP_PORT="${FTP_PORT:-21}"
FTP_USER="${FTP_USER:-anonymous}"
FTP_PASS="${FTP_PASS:-}"
REMOTE_FILE="${REMOTE_FILE:-/releases/hanport-deploy.zip}"
LOCAL_DIR="${LOCAL_DIR:-./downloads}"
LOG_FILE="${LOG_FILE:-./fetchzip.log}"
CLEANUP_ARCHIVE="${CLEANUP_ARCHIVE:-true}"
AUTO_INSTALL="${AUTO_INSTALL:-true}"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --config|-c)
      if [[ -f "$2" ]]; then
        source "$2"
        echo "Loaded config from: $2"
      else
        echo "Error: Config file not found: $2" >&2
        exit 1
      fi
      shift 2
      ;;
    --no-install)
      AUTO_INSTALL=false
      shift
      ;;
    --no-cleanup)
      CLEANUP_ARCHIVE=false
      shift
      ;;
    --help|-h)
      cat <<EOF
FTP Deployment Package Fetcher

Downloads hanport-deploy.zip from FTP, extracts it, and runs install.sh

Usage: $0 [OPTIONS]

Options:
  --config, -c <file>    Load configuration from file
  --no-install           Skip running install.sh after extraction
  --no-cleanup           Keep the archive file after extraction
  --help, -h             Show this help message

Configuration (via file or environment variables):
  FTP_HOST       - FTP server hostname
  FTP_PORT       - FTP server port (default: 21)
  FTP_USER       - FTP username
  FTP_PASS       - FTP password
  REMOTE_FILE    - Path to .zip file on FTP server
  LOCAL_DIR      - Local directory for downloads (default: ./downloads)
  LOG_FILE       - Log file path (default: ./fetchzip.log)
  CLEANUP_ARCHIVE - Remove archive after extraction (default: true)
  AUTO_INSTALL   - Run install.sh after extraction (default: true)

Example config file (ftp.conf):
  FTP_HOST=ftp.example.com
  FTP_PORT=21
  FTP_USER=deploy_user
  FTP_PASS=secret_password
  REMOTE_FILE=/releases/hanport-deploy.zip
  LOCAL_DIR=/tmp/hanport-download
  LOG_FILE=/var/log/hanport-fetch.log

EOF
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      echo "Use --help for usage information" >&2
      exit 1
      ;;
  esac
done

# Logging function
log() {
  local msg="[$(date '+%Y-%m-%d %H:%M:%S')] $*"
  echo "$msg" | tee -a "$LOG_FILE"
}

# Validation
if [[ -z "$FTP_HOST" ]]; then
  log "ERROR: FTP_HOST not specified"
  exit 1
fi

if [[ -z "$REMOTE_FILE" ]]; then
  log "ERROR: REMOTE_FILE not specified"
  exit 1
fi

# Create local directory
mkdir -p "$LOCAL_DIR"
mkdir -p "$(dirname "$LOG_FILE")"

# Extract filename
FILENAME=$(basename "$REMOTE_FILE")
LOCAL_FILE="$LOCAL_DIR/$FILENAME"

log "========================================="
log "FTP Deployment Package Fetcher"
log "========================================="
log "Server:      $FTP_HOST:$FTP_PORT"
log "User:        $FTP_USER"
log "Remote file: $REMOTE_FILE"
log "Local file:  $LOCAL_FILE"
log "Auto-install: $AUTO_INSTALL"
log "Cleanup:     $CLEANUP_ARCHIVE"
log "========================================="

# Check for download tool
if command -v curl &>/dev/null; then
  TOOL="curl"
elif command -v wget &>/dev/null; then
  TOOL="wget"
else
  log "ERROR: Neither curl nor wget found. Install curl or wget."
  exit 1
fi

# Download the file
log "Downloading $FILENAME from FTP..."

if [[ "$TOOL" == "curl" ]]; then
  if [[ -n "$FTP_PASS" ]]; then
    curl -u "$FTP_USER:$FTP_PASS" "ftp://$FTP_HOST:$FTP_PORT$REMOTE_FILE" -o "$LOCAL_FILE" 2>&1 | tee -a "$LOG_FILE"
  else
    curl -u "$FTP_USER:" "ftp://$FTP_HOST:$FTP_PORT$REMOTE_FILE" -o "$LOCAL_FILE" 2>&1 | tee -a "$LOG_FILE"
  fi
else
  # wget
  if [[ -n "$FTP_PASS" ]]; then
    wget --user="$FTP_USER" --password="$FTP_PASS" "ftp://$FTP_HOST:$FTP_PORT$REMOTE_FILE" -O "$LOCAL_FILE" 2>&1 | tee -a "$LOG_FILE"
  else
    wget --user="$FTP_USER" "ftp://$FTP_HOST:$FTP_PORT$REMOTE_FILE" -O "$LOCAL_FILE" 2>&1 | tee -a "$LOG_FILE"
  fi
fi

if [[ $? -eq 0 ]] && [[ -f "$LOCAL_FILE" ]]; then
  log "✓ Download complete: $LOCAL_FILE ($(du -h "$LOCAL_FILE" | cut -f1))"
else
  log "✗ Download failed"
  exit 1
fi

# Extract the zip file
log "Extracting $FILENAME..."

if [[ "$FILENAME" =~ \.zip$ ]]; then
  if command -v unzip &>/dev/null; then
    unzip -o "$LOCAL_FILE" -d "$LOCAL_DIR" 2>&1 | tee -a "$LOG_FILE"
    log "✓ Extracted to: $LOCAL_DIR"
  else
    log "ERROR: unzip not found. Install unzip to extract .zip files."
    exit 1
  fi
else
  log "ERROR: File is not a .zip archive: $FILENAME"
  log "This script only supports .zip files."
  exit 1
fi

# Cleanup archive if requested
if [[ "$CLEANUP_ARCHIVE" == "true" ]]; then
  log "Cleaning up archive file..."
  rm -f "$LOCAL_FILE"
  log "✓ Removed $LOCAL_FILE"
fi

# Find the extracted directory (assume it's hanport-deploy or similar)
EXTRACTED_DIR=$(find "$LOCAL_DIR" -maxdepth 1 -type d -name "hanport-deploy*" | head -1)

if [[ -z "$EXTRACTED_DIR" ]]; then
  log "WARNING: Could not find extracted directory hanport-deploy* in $LOCAL_DIR"
  EXTRACTED_DIR="$LOCAL_DIR"
fi

log "Extracted directory: $EXTRACTED_DIR"

# Run install.sh if present and auto-install is enabled
if [[ "$AUTO_INSTALL" == "true" ]]; then
  INSTALL_SCRIPT="$EXTRACTED_DIR/deploy.sh"
  
  if [[ ! -f "$INSTALL_SCRIPT" ]]; then
    # Try install.sh as fallback
    INSTALL_SCRIPT="$EXTRACTED_DIR/install.sh"
  fi
  
  if [[ -f "$INSTALL_SCRIPT" ]]; then
    log "Running installer: $INSTALL_SCRIPT"
    chmod +x "$INSTALL_SCRIPT"
    
    # Run with sudo (captures output to log)
    sudo bash "$INSTALL_SCRIPT" 2>&1 | tee -a "$LOG_FILE"
    
    if [[ $? -eq 0 ]]; then
      log "✓ Installation complete"
    else
      log "✗ Installation failed (exit code: $?)"
      exit 1
    fi
  else
    log "WARNING: Install script not found at $INSTALL_SCRIPT"
    log "Extracted files are in: $EXTRACTED_DIR"
  fi
else
  log "Auto-install disabled. Extracted files are in: $EXTRACTED_DIR"
  log "To install manually, run: cd $EXTRACTED_DIR && sudo ./deploy.sh"
fi

log "========================================="
log "Done!"
log "========================================="
log "Log saved to: $LOG_FILE"
