# Sprint 9 CI Hotfix Plan – v0.17.1

## Ursache

Der erste Windows-CI-Lauf verwendete MinGW, während kein dazu kompatibles OpenSSL installiert war. CMake brach deshalb bei `find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto)` ab.

## Verbindliche Korrektur

- Die drei Betriebssysteme erhalten getrennte, einfach lesbare Jobs.
- Windows verwendet Qt für MSVC 2022, Visual Studio 2022 und das passende OpenSSL-Windows-Paket.
- Linux verwendet das OpenSSL-Entwicklungspaket der Distribution.
- macOS verwendet `openssl@3` von Homebrew und übergibt dessen Pfad ausdrücklich an CMake.
- Jeder Job baut, testet und paketiert unabhängig.

## Definition of Done

- Keine Vermischung verschiedener Windows-Compiler-Toolchains.
- OpenSSL-Pfad auf jeder Plattform eindeutig.
- Core-Build und Host-Tests weiterhin erfolgreich.
- Windows-, Linux- und macOS-Workflow getrennt nachvollziehbar.
- Vollständige Repository-ZIP und SHA-256-Prüfsumme erstellt.
