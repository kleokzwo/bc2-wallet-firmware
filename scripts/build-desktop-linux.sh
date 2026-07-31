#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build-desktop-linux"
cmake -S "${ROOT}" -B "${BUILD_DIR}" -G Ninja \
  -DBC2_BUILD_SIMULATOR=ON -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --parallel
ctest --test-dir "${BUILD_DIR}" --output-on-failure
cmake --build "${BUILD_DIR}" --target package
printf '\nPakete liegen in: %s\n' "${BUILD_DIR}"
