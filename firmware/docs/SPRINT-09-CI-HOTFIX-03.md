# Sprint 9 CI Hotfix 3 – Windows Generator

## Ursache

Der Windows-Job lief auf `windows-2025`, während CMake ausdrücklich den Generator `Visual Studio 17 2022` verlangte. Auf dem verwendeten Runner konnte CMake keine passende Visual-Studio-Instanz finden.

## Verbindliche Korrektur

- Runner: `windows-2022`
- Compilerumgebung: MSVC x64 über `ilammy/msvc-dev-cmd@v1`
- Buildsystem: Ninja
- Qt: `win64_msvc2022_64`
- OpenSSL: vcpkg `openssl:x64-windows`
- CMake: vcpkg-Toolchain, `CMAKE_BUILD_TYPE=Release`

Dadurch muss CMake keine Visual-Studio-Installation mehr über einen Generator suchen. Compiler, Qt und OpenSSL verwenden dieselbe MSVC-x64-ABI.

## Geltungsbereich

Dieser Hotfix verändert ausschließlich Build- und Release-Infrastruktur. Wallet-Core, Kryptografie, Netzwerk, Gerätefluss und Benutzeroberfläche bleiben funktional unverändert.
