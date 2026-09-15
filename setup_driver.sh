#!/bin/bash
# setup_driver.sh - Paradise Kernel Driver Setup Script
# Uncompress dan load kernel driver untuk DecryptEngine

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DRIVER_ZIP="${SCRIPT_DIR}/driver_ko_601.zip"
DRIVER_DIR="${SCRIPT_DIR}/.driver_extracted"

echo "╔════════════════════════════════════════╗"
echo "║  Paradise Kernel Driver Setup Script   ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   echo "[ERROR] This script must be run as root"
   echo "Usage: sudo ./setup_driver.sh"
   exit 1
fi

# Check if driver zip exists
if [ ! -f "$DRIVER_ZIP" ]; then
    echo "[ERROR] Driver ZIP not found: $DRIVER_ZIP"
    exit 1
fi

# Extract driver
if [ ! -d "$DRIVER_DIR" ]; then
    echo "[*] Extracting driver from $DRIVER_ZIP..."
    mkdir -p "$DRIVER_DIR"
    unzip -q "$DRIVER_ZIP" -d "$DRIVER_DIR"
    echo "[+] Driver extracted to $DRIVER_DIR"
else
    echo "[*] Driver already extracted at $DRIVER_DIR"
fi

# Find .ko file
KO_FILE=$(find "$DRIVER_DIR" -name "*.ko" | head -1)

if [ -z "$KO_FILE" ]; then
    echo "[ERROR] No .ko kernel module found in extracted driver"
    echo "[*] Contents of $DRIVER_DIR:"
    ls -la "$DRIVER_DIR"
    exit 1
fi

echo "[*] Found kernel module: $KO_FILE"
echo ""

# Check if already loaded
MODULE_NAME=$(basename "$KO_FILE" .ko)
if lsmod | grep -q "^$MODULE_NAME "; then
    echo "[!] Module $MODULE_NAME already loaded"
    echo "[*] Unloading previous module..."
    rmmod "$MODULE_NAME" || true
    sleep 1
fi

# Load kernel module
echo "[*] Loading kernel module: $MODULE_NAME"
insmod "$KO_FILE" 2>/dev/null || {
    echo "[ERROR] Failed to load kernel module"
    dmesg | tail -20
    exit 1
}

if [ $? -eq 0 ]; then
    echo "[+] Kernel module loaded successfully"
else
    echo "[ERROR] Failed to load kernel module"
    exit 1
fi

# Verify
sleep 1
if lsmod | grep -q "^$MODULE_NAME "; then
    echo "[+] ✓ Verification: Module is loaded and ready"
    echo ""
    lsmod | grep "$MODULE_NAME"
    echo ""
else
    echo "[ERROR] Module verification failed"
    exit 1
fi

echo "[+] Setup complete! Paradise driver is ready."
echo ""
echo "Next steps:"
echo "  1. chmod +x build.sh"
echo "  2. ./build.sh"
echo "  3. Start game on device: com.proximabeta.mf.uamo"
echo "  4. ./build/decrypt_engine com.proximabeta.mf.uamo"
echo ""

exit 0
