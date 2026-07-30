@echo off
setlocal
cmake -S . -B build -DBC2_BUILD_SIMULATOR=OFF -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
cmake --build build --config Release --parallel
if errorlevel 1 exit /b 1
ctest --test-dir build -C Release --output-on-failure
