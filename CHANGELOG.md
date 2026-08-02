## 0.22.0

- Korrigiert die E-Paper-BUSY-Wartefunktion für FreeRTOS-Konfigurationen mit 100-Hz-Tick.
- Verwendet `esp_timer_get_time()` für echte Timeout-Messung statt eines hochgezählten Schleifenzählers.
- Erzwingt während BUSY mindestens einen echten Scheduler-Tick und verhindert dadurch falsche 15-Sekunden-Timeouts nach nur etwa 30 ms.
- Firmware-, Desktop- und Geräteprotokollversion auf 0.22.0 erhöht.

## 0.21.4

- Offiziellen Waveshare-V2-E-Paper-BSP portiert
- Richtige V2-Pins inklusive EPD-Power GPIO 6 aktiviert
- Offizielle Vollrefresh-LUT und Refresh-Sequenz integriert
- BC2-Anzeige bleibt über die bestehende HAL entkoppelt

# Changelog

## 0.21.4

- korrigierte BUSY-Polarität für das Waveshare ESP32-S3-ePaper-1.54 V2 (aktiv HIGH)
- echter zweiphasiger Refresh-Nachweis: BUSY muss HIGH werden und wieder LOW werden
- Vollrefresh-Steuerbyte auf `0xF7` und Border-Waveform auf `0x05` umgestellt
- detaillierte Refresh-Diagnose und belastbare Fehlermeldungen ergänzt
- V2-GPIO-Profil, BOOT-Taste, USB, NVS und RNG aus v0.21.2 beibehalten
- Firmware- und Projektversion auf 0.21.4 erhöht

## 0.20.0 - 2026-07-31

### Added
- Neues C17-Modul für Transaktionsplanung und unsigned PSBT-Erzeugung.
- BC2-Mainnet-Adressprüfung für SegWit v0 und P2PKH.
- Deterministische Coin Selection aus synchronisierten Watch-only-UTXOs.
- Gebühren-, Dust- und Wechselgeldberechnung.
- Send-Maske mit Empfänger, Betrag, Gebührenrate, Vorschau und PSBT-Export.
- End-to-End-Test über Erzeugung und bestehenden PSBT-Parser.
- Verbindlicher Sprint-11-Plan und Abschlussbericht.

### Changed
- Transaktionsseite von einer reinen PSBT-Prüfung zu einer Send-Vorbereitung erweitert.
- Versionsnummer auf 0.20.0 erhöht.

### Security
- PSBTs werden ausschließlich unsigned erzeugt.
- Kein Seed, kein Private Key, keine Signierung und kein Broadcast im Desktop.
- Hardwarebestätigung bleibt bis zur physischen Hardware deaktiviert.

## 0.18.0 - 2026-07-31

### Added
- Robuste Watch-only-Verbindungssteuerung mit manuellem Disconnect.
- Automatische Wiederverbindung und Request-Timeouts.
- Blockhöhen-Abonnement über Electrum.
- Automatische Synchronisation alle fünf Minuten.
- Getrennte bestätigte und unbestätigte Kontostände.
- Transaktionsverlauf mit Status, TXID, bekanntem Betrag und Bestätigungen.
- Vorbereitete Multi-Account- und Multi-XPUB-Datenstrukturen.

### Security
- Keine Seeds oder privaten Schlüssel werden gespeichert.
- Keine Signierung wurde dem Watch-only-Pfad hinzugefügt.

## 0.17.7 - Windows packaging hotfix

- Fixed Qt deployment packaging on Windows by removing spaces from the executable file name.
- The executable is now `BC2-Cold-Wallet.exe`; the visible application and bundle name remains “BC2 Cold Wallet”.
- Prevents `qt_deploy_runtime_dependencies` from interpreting parts of the executable name as unsupported arguments.
- No wallet-core, cryptographic, network, seed, signing, or hardware behavior changed.

## v0.17.7 – Qt/MSVC Compile Hotfix

- Added the missing `QDesktopServices` include in `mainwindow_pages.cpp`.
- Added the BC2 explorer URL in the translation unit that uses it.
- Renamed the local `pageChanged` boolean to avoid shadowing the Qt signal on MSVC.
- No wallet-core, cryptography, seed, signing, networking, or hardware behavior changed.

## v0.17.5

- Windows release build no longer treats compiler warnings as fatal errors.
- MSVC remains configured with warning level `/W4`.
- Windows test targets explicitly undefine `NDEBUG` so assertions remain active.
- Native desktop builds now use verbose compiler output for reliable diagnostics.
- No wallet behavior, cryptography, network, seed, signing, or hardware logic changed.

## v0.17.5 – Windows C17 Portability Hotfix

- Gemeinsame KISS-Kompatibilitätsschicht `bc2_compat.h` ergänzt.
- POSIX-`strtok_r` wird unter MSVC sicher auf `strtok_s` abgebildet.
- Unsichere `strcpy`-Aufrufe durch eine geprüfte, größenbegrenzte Kopierfunktion ersetzt.
- OpenSSL-3-Deprecation-Attribute für den bestehenden, getesteten EC-Code zentral unterdrückt; `/W4 /WX` bleibt für den C17-Core aktiv.
- Keine Wallet-Funktion, Ableitung, Signaturausgabe oder Sicherheitsgrenze verändert.
- Linux-Host-Build und 13/13 Tests erfolgreich.

## v0.17.2 – Sprint 9 CI Hotfix 2

- Windows OpenSSL-Erkennung auf vcpkg/CMake-Toolchain umgestellt.
- Feste Chocolatey-Installationspfade entfernt.
- macOS-OpenSSL-Pfade explizit aus Homebrew exportiert.
- Linux-Abhängigkeiten für Qt ergänzt.
- Plattformbezogene CI-Diagnose bei Fehlern ergänzt.
- Qt-Frontend-Warnungen bleiben aktiv, blockieren aber keine plattformübergreifenden Release-Builds mehr.
- Qt-Deployment auf Windows und macOS begrenzt.

## v0.17.2 – Desktop CI Hotfix

- Windows-Build von gemischter MinGW-Konfiguration auf eine konsistente MSVC-2022-Toolchain umgestellt.
- OpenSSL wird auf Windows installiert und über `OPENSSL_ROOT_DIR` eindeutig gefunden.
- Linux, Windows und macOS besitzen getrennte, KISS-konforme CI-Jobs.
- macOS verwendet den expliziten Homebrew-Pfad für OpenSSL 3.
- Checkout-Action auf die Node-24-kompatible Hauptversion aktualisiert.
- Keine Wallet- oder Sicherheitslogik verändert.

## 0.16.0

- Added central KISS-oriented desktop settings storage.
- Persisted theme, window geometry and Electrum connection settings.
- Kept all private data, seeds and signing outside desktop settings.
- Added Sprint 8 plan and report.


## v0.15.0 – Sprint 7: Waveshare Bring-up

- Waveshare-BSP nach KISS in kleine Funktionen zerlegt
- USB Serial/JTAG als BC2-HAL-Transport angebunden
- gemeinsamer C17-Geräteservice für Ping, Geräteinfo und Gerätezustand
- Firmware- und Boardidentität über das bestehende USB-Protokoll
- Display- und Button-Treiber bleiben bis zur bestätigten Pinbelegung gesperrt
- neuer `test_device_service`; 13/13 Host-Tests bestanden
- Sprint-Plan, Sprint-Report, README und Testnachweis aktualisiert

## 0.14.0

- Added the shared C17 `bc2_device_flow` layer for simulator and ESP32.
- Added centralized device-state to e-paper-screen mapping.
- Added centralized hardware-button to device-event mapping.
- Updated the ESP32 entry point to run the shared state machine and device flow.
- Added Qt controller support for the shared hardware-button mapping.
- Added `test_device_flow`; all 12 host tests pass.
- Added the binding Sprint 6 plan and completion report.
- Updated project and simulator version to 0.14.0.
- Kept Wi-Fi, Bluetooth, signing, seed handling and unverified GPIO drivers disabled.

## 0.13.0

- Added active-page highlighting to the shared Qt navigation.
- Added compact responsive symbol navigation with tooltips and accessible names.
- Added a scroll-safe sidebar for smaller window heights.
- Added centralized 160 ms page fade transitions in PageRouter.
- Polished focus, hover, input, scrollbar and tooltip states in both themes.
- Added a visible development-environment badge.
- Added the binding Sprint 5 plan and completion report.
- Updated project and simulator version to 0.13.0.
- Preserved Wallet Core, cryptography, network, device-state, HAL and security behavior.

## 0.12.0

- Added separate routed Qt pages for Transaction, History, Settings, About, Recovery, Backup, Error and Factory Reset.
- Connected the existing PSBT inspector through the Transaction page.
- Kept recovery seed entry, signing and factory reset disabled on desktop.
- Added the binding Sprint 4 plan and completion report.
- Updated project and simulator version to 0.12.0.
- Preserved Wallet Core, network, cryptography, device-state and HAL behavior.

## 0.11.0

### Added
- Reusable Qt components: Bc2Button, Bc2Card, Bc2Header, Bc2Dialog, Bc2StatusBar, Bc2LoadingWidget and Bc2QrWidget.
- Central component styles for page titles, section titles, status states and destructive actions.
- Binding Sprint 3 development plan and completion report.

### Changed
- Existing simulator pages now use the shared button, card and header components.
- Simulator and project version updated to 0.11.0.

### Security
- Scan-capable QR generation remains deferred until it can be integrated and tested with the complete Receive page; no unvalidated encoder was introduced.

## 0.10.0
- Sprint 2 Modernes Qt-Grundgerüst abgeschlossen.
- `PageRouter` für benannte, indexfreie Seitennavigation ergänzt.
- zentralen `ThemeManager` mit Dark Mode und Light Mode eingeführt.
- zentrale `DesignTokens` für responsive Größen, Abstände und Radien ergänzt.
- Sidebar reagiert auf kleinere Desktop-Fenster mit kompakter Breite.
- globale Styles aus `MainWindow` entfernt.
- Wallet-Core erfolgreich gebaut; 11/11 Host-Tests bestanden.
- Qt-Build mangels installierter Qt-6-Entwicklungsumgebung nicht ausführbar.

## 0.9.1
- Sprint 1 Repository Cleanup ohne neue Produktfunktionen abgeschlossen.
- Qt-MainWindow in Verhaltenslogik und separaten Seitenaufbau aufgeteilt.
- `mainwindow.cpp` von 551 auf 341 Zeilen reduziert.
- Navigation lesbarer strukturiert und veraltete Simulator-Versionsanzeige korrigiert.
- Development Master Plan und Sprint-Report ergänzt.
- Wallet-Core erfolgreich gebaut; 11/11 Host-Tests bestanden.

## 0.9.0
- Zielhardware Waveshare ESP32-S3-ePaper-1.54 verbindlich aufgenommen.
- Displayreferenz auf 200×200 korrigiert.
- ESP-IDF-5.5-Projekt und ESP32-S3-BSP angelegt.
- ESP-Timer, Hardware-RNG und NVS an die gemeinsame HAL angebunden.
- Begrenztes USB-Protokoll v1 und Test ergänzt.
- Funk bleibt deaktiviert; Display-/Tasten-GPIOs warten auf bestätigte V1/V2-Revision.

## [0.17.0] - 2026-07-31

### Added
- Reproduzierbare Desktop-Paketierung mit CMake und CPack.
- Qt-Deployment für portable Desktop-Pakete.
- Native GitHub-Actions-Builds für Linux, Windows und macOS.
- Lokale Build-Skripte für Linux und Windows.
- Vollständige plattformübergreifende Desktop-Build-Dokumentation.

### Unchanged
- Keine Änderungen an Wallet-Core, Kryptografie, Seed, PIN, Signing oder Hardware-Wire-up.

## v0.22.0

- Replaced the custom E-paper implementation with the original Waveshare V2 `epaper_driver_bsp` and `board_power_bsp` sources from the hardware-confirmed LVGL9 demo.
- Added a small BC2 display adapter; wallet and device logic no longer knows SPI, LUT, BUSY, or panel commands.
- Uses the same startup and partial-refresh path that visibly worked on the physical ESP32-S3-ePaper-1.54.
- Retains USB, NVS, RNG, device state, and BOOT-button integration.
