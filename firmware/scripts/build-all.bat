@echo off
setlocal
set ROOT=%~dp0..
cmake -S "%ROOT%" -B "%ROOT%\build" -DBC2_BUILD_SIMULATOR=ON -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
cmake --build "%ROOT%\build" --config Release --parallel
if errorlevel 1 exit /b 1
ctest --test-dir "%ROOT%\build" -C Release --output-on-failure
