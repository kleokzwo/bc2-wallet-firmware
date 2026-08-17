# BC2 Cold Wallet – Desktop Build Guide

Version: 0.17.0

## Ziel

Diese Anleitung erzeugt testbare Desktop-Pakete für Linux, Windows und macOS. Der Wallet-Core bleibt auf allen Plattformen identisch.

## Wichtige technische Grenze

Linux kann Linux-Pakete direkt bauen. Windows-Cross-Builds sind mit einer vollständig vorbereiteten MXE/MinGW-Qt-Toolchain möglich, aber deutlich wartungsintensiver. Ein korrekt signierbares macOS-App-Bundle kann nicht seriös mit einer normalen Linux-Toolchain erzeugt werden, weil dafür Apple SDK, Xcode und ein macOS-Runner nötig sind.

Der verbindliche KISS-Weg ist deshalb:

1. lokal unter Linux entwickeln und testen;
2. GitHub Actions erzeugt native Linux-, Windows- und macOS-Pakete;
3. das Windows-ZIP wird ohne Installation entpackt und gestartet.

## Voraussetzungen unter Linux

- CMake 3.20 oder neuer
- Ninja
- C/C++-Compiler
- OpenSSL 3 Development Files
- Qt 6.4 oder neuer mit Widgets, Network und SerialPort

Ubuntu/Debian:

```bash
sudo apt update
sudo apt install cmake ninja-build build-essential libssl-dev qt6-base-dev qt6-serialport-dev
```

## Linux lokal bauen

```bash
./scripts/build-desktop-linux.sh
```

Das Skript konfiguriert, baut, führt alle Tests aus und erzeugt anschließend ein Paket im Ordner `build-desktop-linux`.

## Windows auf einem Windows-Rechner bauen

Benötigt werden CMake, Ninja, Qt 6 und OpenSSL 3. Danach:

```bat
scripts\build-desktop-windows.bat
```

Das erzeugte ZIP liegt unter `build-desktop-windows`.

## Windows-App von einer Linux-Maschine erhalten

Der empfohlene Weg ist der bereits enthaltene Workflow:

```text
.github/workflows/desktop-release.yml
```

Ablauf:

1. Repository zu GitHub pushen.
2. Unter **Actions → Desktop Build → Run workflow** starten.
3. Nach Abschluss das Artefakt `bc2-cold-wallet-Windows` herunterladen.
4. ZIP auf dem Windows-Rechner entpacken.
5. `BC2 Cold Wallet.exe` starten.

Dafür werden auf dem Windows-Testrechner keine Administratorrechte benötigt.

## macOS-Paket von der Linux-Entwicklungsmaschine auslösen

Der gleiche Workflow startet einen nativen macOS-Runner und erzeugt ein DMG. Das Build wird von Linux aus über GitHub angestoßen, läuft technisch aber auf macOS. Das ist notwendig, damit das Apple SDK korrekt verwendet wird.

## Release auslösen

Ein Tag startet alle drei Builds automatisch:

```bash
git tag v0.17.0
git push origin v0.17.0
```

Alternativ kann der Workflow manuell gestartet werden.

## Sicherheitsregeln

Build-Pakete dürfen keine Seeds, PINs, privaten Schlüssel, Test-Wallets oder lokale Einstellungsdateien enthalten. Release-Pakete werden nur aus eingechecktem Quellcode erzeugt.

## Fehlerdiagnose

### Qt6Config.cmake nicht gefunden

`CMAKE_PREFIX_PATH` auf die Qt-Installation setzen oder Qt über den offiziellen Installer installieren.

### OpenSSL nicht gefunden

Unter Linux `libssl-dev`, unter macOS `openssl@3` und unter Windows eine passende OpenSSL-3-Entwicklungsinstallation bereitstellen.

### Windows startet nicht wegen fehlender DLL

Nur das vollständige CI-ZIP verwenden. Der Qt-Deploy-Schritt kopiert die benötigten Laufzeitbibliotheken in das Paket.
