# Sprint 9 Report – Desktop Build & Release

## Ergebnis

Version 0.17.0 ergänzt eine reproduzierbare Desktop-Release-Struktur. CMake installiert die Desktop-App und verwendet Qt Deployment, damit Plattformpakete die benötigten Qt-Laufzeitdateien enthalten. CPack erzeugt portable Release-Dateien.

## Neue Dateien

- `scripts/build-desktop-linux.sh`
- `scripts/build-desktop-windows.bat`
- `.github/workflows/desktop-release.yml`
- `docs/DESKTOP-BUILD-GUIDE.md`
- `docs/SPRINT-09-DEVELOPMENT-PLAN.md`

## Plattformstrategie

- Linux: lokaler nativer Build
- Windows: nativer GitHub-Actions-Build und optional lokaler Windows-Build
- macOS: nativer GitHub-Actions-Build, da Apple SDK und Xcode benötigt werden

## Sicherheit

Wallet-Core und sicherheitskritische Funktionen wurden nicht verändert. Pakete enthalten keine Wallet-Geheimnisse.
