#!/bin/bash
set -euo pipefail

echo "========================================"
echo "SmartSchedule Local CI/CD Verification"
echo "========================================"

echo "[1] Clean & Configure"
rm -rf build-local-ci
cmake --preset ci-ubuntu -B build-local-ci

echo "[2] Build"
cmake --build build-local-ci --parallel

echo "[3] Test"
ctest --test-dir build-local-ci --output-on-failure

echo "[4] Format Check"
clang-format -i src/**/*.cpp src/**/*.h tests/**/*.cpp 2>/dev/null || true
if ! git diff --quiet src/ tests/; then
    echo "Format check failed: files were modified by clang-format"
    git checkout src/ tests/
    exit 1
fi

echo "[5] Package"
cd build-local-ci
cpack -C Release

echo "[6] Checksum"
sha256sum *.deb > SHA256SUMS.txt
sha256sum -c SHA256SUMS.txt

echo "========================================"
echo "LOCAL CI READY"
echo "========================================"
