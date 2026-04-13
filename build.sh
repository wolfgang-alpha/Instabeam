#!/bin/bash
set -e

BUILD_DIR="build"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "=== Running qmake ==="
qmake ../TrussCalculator.pro

echo "=== Building ==="
make -j$(nproc)

echo "=== Build complete ==="
echo "Binary: $(pwd)/TrussCalculator"
