# Sprint 9 CI Hotfix Report – v0.17.1

## Behobener Fehler

Der Windows-Job konfigurierte das Projekt mit MinGW. Die CI-Umgebung enthielt jedoch keine kompatible OpenSSL-Crypto-Bibliothek. Dadurch konnte CMake `OPENSSL_CRYPTO_LIBRARY` und `OPENSSL_INCLUDE_DIR` nicht finden.

## Umsetzung

1. Der gemeinsame Matrix-Job wurde durch drei eigenständige Jobs ersetzt.
2. Windows verwendet nun durchgehend MSVC 2022:
   - Qt `win64_msvc2022_64`
   - Visual-Studio-CMake-Generator
   - OpenSSL-Win64
3. macOS übergibt den Homebrew-Pfad für `openssl@3` ausdrücklich.
4. Linux installiert `libssl-dev` ausdrücklich.
5. Checkout wurde auf die Node-24-kompatible Hauptversion aktualisiert.

## Sicherheitsauswirkung

Keine Wallet-, Seed-, Signatur-, Netzwerk- oder Hardwarelogik wurde verändert. Die Änderung betrifft ausschließlich Desktop-CI und Paketierung.

## Lokaler Nachweis

Der C17-Core, die CLI und alle 13 Host-Tests wurden nach der Änderung erneut gebaut und ausgeführt. Der tatsächliche native GUI-Paketbau muss durch die drei GitHub-Runner bestätigt werden.
