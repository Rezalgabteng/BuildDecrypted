#!/bin/bash
# build.sh - Quick build script for BuildDecrypted

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

echo "╔════════════════════════════════════════╗"
echo "║     BuildDecrypted Build Script        ║"
echo "║     Target: com.proximabeta.mf.uamo    ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Check prerequisites
echo "[*] Checking prerequisites..."
if ! command -v cmake &> /dev/null; then
    echo "[ERROR] cmake not found. Install with:"
    echo "  sudo apt-get install cmake"
    exit 1
fi

if ! command -v g++ &> /dev/null; then
    echo "[ERROR] g++ not found. Install with:"
    echo "  sudo apt-get install build-essential"
    exit 1
fi

echo "[+] Prerequisites OK (cmake, g++)"
echo ""

# Check required files
echo "[*] Checking required files..."
if [ ! -f "$SCRIPT_DIR/libparadise_api.a" ]; then
    echo "[WARNING] libparadise_api.a not found"
fi

if [ ! -f "$SCRIPT_DIR/DecryptEngine.hpp" ]; then
    echo "[ERROR] DecryptEngine.hpp not found"
    exit 1
fi

echo "[+] Required files present"
echo ""

# Create build directory
if [ ! -d "$BUILD_DIR" ]; then
    echo "[*] Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

# Run CMake
echo "[*] Running CMake..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo ""
echo "[*] Compiling (using $(nproc) cores)..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo ""
    echo "╔════════════════════════════════════════╗"
    echo "║         Build Successful! ✓            ║"
    echo "╚════════════════════════════════════════╝"
    echo ""
    echo "Output: ${BUILD_DIR}/decrypt_engine"
    echo ""
    echo "Next steps:"
    echo "  1. Setup kernel driver:"
    echo "     sudo ${SCRIPT_DIR}/setup_driver.sh"
    echo ""
    echo "  2. Start game on device:"
    echo "     com.proximabeta.mf.uamo"
    echo ""
    echo "  3. Run coordinate extractor:"
    echo "     ${BUILD_DIR}/decrypt_engine com.proximabeta.mf.uamo"
    echo ""
else
    echo ""
    echo "╔════════════════════════════════════════╗"
    echo "║          Build Failed ✗               ║"
    echo "╚════════════════════════════════════════╝"
    exit 1
fi
