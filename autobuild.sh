#!/bin/bash
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
INCLUDE_INSTALL_DIR="/usr/include/mymuduo"
LIB_INSTALL_DIR="/usr/lib"
LIB_NAME="libmy_muduo.so"

mkdir -p "$BUILD_DIR"
rm -rf "${BUILD_DIR:?}"/*
cd "$BUILD_DIR"
cmake ..
make -j"$(nproc)"
cd "$PROJECT_ROOT"

echo "Installing headers to $INCLUDE_INSTALL_DIR..."
sudo mkdir -p "$INCLUDE_INSTALL_DIR"
HEADER_DIR="${PROJECT_ROOT}/include/mymuduo"
if [ -d "$HEADER_DIR" ]; then
    for header in "$HEADER_DIR"/*.h; do
        [ -e "$header" ] && sudo cp "$header" "$INCLUDE_INSTALL_DIR/"
    done
fi

if [ -f "${PROJECT_ROOT}/lib/${LIB_NAME}" ]; then
    echo "Installing library to $LIB_INSTALL_DIR..."
    sudo cp "${PROJECT_ROOT}/lib/${LIB_NAME}" "$LIB_INSTALL_DIR/"
    sudo ldconfig
else
    echo "Warning: ${PROJECT_ROOT}/lib/${LIB_NAME} not found."
fi

echo "Build and installation completed."