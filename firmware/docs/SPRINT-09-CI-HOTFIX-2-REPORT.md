# Sprint 9 CI Hotfix 2 – Abschlussbericht

## Behobene Ursache

v0.17.1 erwartete Chocolatey-OpenSSL fest unter `C:/Program Files/OpenSSL-Win64`. Der GitHub-Runner installierte das Paket nicht an diesem garantierten Ort. Der Configure-Schritt wurde deshalb absichtlich mit Exit-Code 1 beendet.

## Änderungen

- Windows OpenSSL-Installation auf vcpkg umgestellt.
- CMake erhält die vcpkg-Toolchain und das Triplet `x64-windows`.
- macOS exportiert `OPENSSL_ROOT_DIR`, `CMAKE_PREFIX_PATH` und `PKG_CONFIG_PATH` aus Homebrew.
- Linux installiert zusätzlich die OpenGL-Entwicklungsabhängigkeit für Qt.
- Alle Plattformjobs geben bei Fehlern relevante Logdateien aus.
- Warnungen bleiben im C17-Core Fehler; das Qt-Frontend bleibt mit hohen Warnstufen aktiv, blockiert Releases aber nicht wegen plattformspezifischer Warnungen.
- Qt-Deployment-Skript wird nur unter Windows und macOS installiert.

## Lokale Prüfung

Der plattformunabhängige Core, die CLI und alle 13 Host-Tests wurden erfolgreich geprüft. Die drei nativen GUI-Pakete müssen durch die jeweiligen GitHub-hosted Runner bestätigt werden.
