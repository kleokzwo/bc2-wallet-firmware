## v0.41.0

See `CHANGELOG-v0.41.0.md`.

## 0.29.7

- USB-Empfang auf dem ESP32-S3 ist wieder nichtblockierend; die Firmware wartet
  nicht mehr bis zu 250 ms pro Loop, wenn keine Daten vorliegen.
- Hardwaretasten werden vor jeder USB-Verarbeitung abgefragt, damit PIN- und
  Bestätigungsdrücke auch bei laufenden Desktop-Abfragen zuverlässig ankommen.
- Nur das tatsächliche USB-Senden besitzt eine kurze, auf 20 ms begrenzte
  Schreibfrist.
- Regressionstest schützt die Priorität der physischen Eingabe und den
  nichtblockierenden USB-Empfang.
- Hardware löscht `APPROVED` oder `REJECTED` nicht mehr beim bloßen Erstellen
  einer USB-Antwort. Das Endergebnis bleibt bis zur nächsten gültigen
  Transaktionsanfrage wiederholbar abrufbar.
- USB-Senden auf dem ESP32-S3 besitzt eine kurze begrenzte Schreibfrist.
- Der Simulator verwirft vor einer neuen Abfrage keine verspäteten Frames mehr
  und fragt während blockierender E-Paper-Aktualisierungen weiter ab.
- Regressionstest belegt, dass eine bestätigte Entscheidung bei mehrfacher
  Ergebnisabfrage erhalten bleibt.
- `ZAHLUNG FREIGEBEN` verwendet zuverlässig das PIN-Layout.

## 0.29.3

- Falsches, fest codiertes PSBT-Adresspräfix `bc2` entfernt.
- Der Simulator verwendet jetzt dieselbe zentrale BC2-Mainnet-Konfiguration
  (`bc`) wie Wallet-Core und Hardware.
- Die mitgelieferte Test-PSBT muss im Regressionstest eine Adresse erzeugen,
  die auch die echte Hardwarevalidierung akzeptiert.
- Lebenszyklus der Hardwareprüfung korrigiert: eine entnommene Anfrage bleibt
  bis zur physischen Entscheidung ausdrücklich aktiv.
- Verhindert, dass Warteschlangenstatus und tatsächlicher Prüfstatus
  miteinander verwechselt werden.
- Abgeschlossene, noch nicht abgeholte Ergebnisse blockieren keine neue Prüfung.
- Zweitanfragen während PIN-Eingabe oder Transaktionsanzeige liefern jetzt
  ausdrücklich „Prüfung läuft“ statt einer allgemeinen Ablehnung.
- Regressionstest für PIN-/Prüfphase und direkt folgende neue Prüfung ergänzt.

## 0.29.1

- Festhängenden Prüfstatus nach abgeschlossener oder unterbrochener Prüfung behoben.
- Direkt aufeinanderfolgende Hardwareprüfungen funktionieren wieder; während
  einer tatsächlich aktiven Prüfung bleibt eine zweite Anfrage gesperrt.
- Verbindlichen Ergebnisabruf für die Hardware-Transaktionsprüfung ergänzt.
- Simulator meldet Erfolg erst nach der physischen Bestätigung auf dem Gerät.
- BOOT-Taste lehnt eine angezeigte Transaktion ausdrücklich ab.
- Geräteablehnung, USB-Verbindungsverlust und 120-Sekunden-Timeout werden
  eindeutig unterschieden.
- Parallele Transaktionsprüfungen werden während einer offenen Entscheidung
  abgelehnt.
- C17-Geräteservice- und Geräteflow-Tests um Ergebniszustände und Abbruch ergänzt.
- Keine Signierung, kein Broadcast und keine Änderung am Waveshare-BSP.

## 0.28.0

- Neuen USB-Befehl `REVIEW_TRANSACTION` mit begrenztem, versioniertem Format ergänzt.
- Gerät validiert BC2-Empfänger, Beträge, Überläufe sowie bestätigte Gebühren-
  und Wechselgeldkennzeichen.
- PSBT-Dialog bietet die Hardwareprüfung nur für vollständig geprüfte
  Transaktionen mit genau einem externen Empfänger an.
- Erneute Geräte-PIN-Freigabe und physische Transaktionsbestätigung ergänzt.
- Empfänger, Betrag, Gebühr und Wechselgeld werden vollständig auf dem E-Paper angezeigt.
- Keine Signierung, kein Broadcast und keine Übertragung von Seed, Private Key oder PIN.
- Geräte-Service-Test um gültige und abgelehnte Transaktionsanfragen erweitert.
- Offizieller Waveshare-Treiber und BSP unverändert belassen.

## 0.27.2

- Verbessert die E-Paper-Reinigung beim Wechsel zwischen Dashboard, PIN und Adresspruefung.
- Neue Ansichten erzwingen einen Full-Refresh und setzen danach eine neue Partial-Basis.
- Nach hoechstens 12 Partial-Updates erfolgt vorsorglich ein Full-Refresh.
- Offizieller Waveshare-Treiber und BSP bleiben unveraendert.
- Der Geraeteservice-Test verwendet die zentrale Firmware-Version statt eines
  fest codierten alten Versionsstrings.
- Hinweis: Vollrefreshes sind bauartbedingt langsam; verbleibendes Ghosting ist
  auf echter Hardware zu bewerten und nicht pauschal als vollstaendig geloest markiert.

## 0.27.1

- Macht die automatische USB-Erkennung unter Linux robuster.
- Prüft typische USB-Seriell-Ports zuerst und wiederholt den Geräte-Ping.
- Zeigt an, ob ein Anschluss belegt, nicht zugreifbar oder ohne Antwort ist.

## 0.27.0

- Desktop-Simulator erkennt das BC2-Gerät automatisch über USB-Serial.
- Der Empfangen-Button überträgt die aktuell angezeigte Adresse direkt an die Hardware.
- Hardware-Ablehnungen und fehlende bzw. belegte serielle Anschlüsse werden verständlich angezeigt.
- Die Hardware bleibt alleinige Instanz für Adressvalidierung, Gerätezustand und PIN-Freigabe.
- Manuelle Eingabe von COM- oder `/dev/ttyACM`-Anschlüssen entfällt.

## 0.26.0

- Fuegt den USB-Befehl `REVIEW_RECEIVE_ADDRESS` hinzu.
- Nimmt Empfangsadressen nur im entsperrten Dashboard an.
- Validiert BC2-Mainnet-Bech32-P2WPKH inklusive Pruefsumme auf dem Geraet.
- Fordert fuer jede Empfangsadressen-Pruefung erneut den Geraete-PIN an.
- Zeigt die vollstaendige Adresse ohne abgeschnittene Zeichen auf dem E-Paper.
- Lehnt Testnet-, fehlerhafte und gemischt/gross geschriebene Adressen ab.
- Erweitert das USB-Hilfsprogramm um `--receive-address`.
- Belaesst Waveshare-BSP, E-Paper-Herstellertreiber und Power-BSP unveraendert.

## 0.25.2

- Verhindert IDLE0-Watchdog-Resets waehrend PBKDF2-HMAC-SHA-256.
- Behaelt das kompatible PBKDF2-Ergebnis und 100.000 KDF-Runden bei.
- Gibt dem ESP32-S3-Scheduler waehrend der Ableitung regelmaessig Zeit.
- Waveshare-BSP und Displaytreiber bleiben unveraendert.

## 0.25.1

- ESP32-S3-Stackueberlauf beim Start behoben.
- BC2-Anwendung laeuft in einem eigenen FreeRTOS-Task mit 16 KiB Stack.
- Waveshare-BSP und Displaytreiber bleiben unveraendert.

## 0.25.0

- Festen Entwicklungstest-PIN `2468` aus der Hardware-App entfernt.
- Sechsstellige PIN-Anlage mit Wiederholung beim ersten Start hinzugefuegt.
- Gesalzenen PBKDF2-HMAC-SHA-256-PIN-Verifier mit 100.000 Runden implementiert.
- Persistente Fehlversuchszaehlung und progressive Sperrzeiten hinzugefuegt.
- Geräte-/Root-PIN-Autorisierungsregeln zentral definiert.
- PIN-Anzeige auf sechs Stellen erweitert.
- Neuen Security-Test hinzugefuegt und bestehende PIN-Tests aktualisiert.
- Versionsnummer auf 0.25.0 erhoeht.
- Originalen Waveshare-BSP und beide Herstellerkomponenten unveraendert gelassen.

## 0.24.0

- Zwei-Tasten-Navigation für PWR/BAT_KEY (GPIO 18) und BOOT (GPIO 0) ergänzt.
- Gleichzeitiges Drücken beider Tasten als eindeutige Bestätigung implementiert.
- Taschenrechnerartige PIN-Ansicht mit Ziffern, verdeckter Eingabe und `[<]`-Löschtaste ergänzt.
- Plattformunabhängigen C17-PIN-Controller und Tests hinzugefügt.
- Test-PIN `2468` für den nicht produktiven Hardware-UI-Test übernommen.
- Versionsnummer auf 0.24.0 erhöht.
- Displaytreiber, Refresh-Sequenzen und BUSY-Behandlung unverändert gelassen.

## 0.23.0

- Neues Hardware-UI für den Sperrbildschirm mit BC2-Branding, Statusleiste, Schloss-, USB- und Batteriesymbol.
- KISS-konformen Navigation Controller für Kurz- und Langdruck ergänzt.
- Langer Druck sperrt einen entsperrten Gerätezustand; kurzer Druck bleibt die Bestätigung.
- Bestehendes Full-Refresh-Flag mit dem offiziellen Waveshare-Anzeigepfad verbunden.
- Navigation durch einen eigenständigen Host-Test abgesichert.
- Firmware-, Desktop- und Geräteprotokollversion auf 0.23.0 erhöht.
- Originalen Waveshare-BSP, GPIO-Mapping und BUSY-Handling unverändert gelassen.

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
