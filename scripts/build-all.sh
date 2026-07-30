#!/usr/bin/env sh
set -eu
cmake -S . -B build -DBC2_BUILD_SIMULATOR=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
