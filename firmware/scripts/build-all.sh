#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmake -S "$root" -B "$root/build" -DCMAKE_BUILD_TYPE=Release -DBC2_BUILD_SIMULATOR=ON
cmake --build "$root/build" --parallel
ctest --test-dir "$root/build" --output-on-failure
