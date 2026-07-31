@echo off
setlocal
set ROOT=%~dp0..
set BUILD_DIR=%ROOT%\build-desktop-windows
cmake -S "%ROOT%" -B "%BUILD_DIR%" -G Ninja -DBC2_BUILD_SIMULATOR=ON -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
cmake --build "%BUILD_DIR%" --parallel
if errorlevel 1 exit /b 1
ctest --test-dir "%BUILD_DIR%" --output-on-failure
if errorlevel 1 exit /b 1
cmake --build "%BUILD_DIR%" --target package
if errorlevel 1 exit /b 1
echo Pakete liegen in: %BUILD_DIR%
