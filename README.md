# BC2 Cold Wallet

## Version 0.17.2

Die aktuelle Version schließt Sprint 7 als sichere Waveshare-Hardware-Bring-up-Basis ab. Der ESP32-S3 besitzt jetzt einen getesteten BC2-USB-Transport für Geräteinformationen und Zustandsabfragen; unbekannte Display- und Button-Pins bleiben bewusst gesperrt.


## Neu in v0.16.0

- KISS-Refactoring des Waveshare-BSP
- USB Serial/JTAG als HAL-Transport für das gerahmte BC2-Protokoll
- gemeinsamer C17-Geräteservice mit `PING`, `GET_INFO` und `GET_STATE`
- getrennte USB-Protokoll- und Logkanäle
- explizite Hardware-Bereitschaft für Display und Buttons
- keine erfundenen GPIO-Pinbelegungen
- neuer Geräteservice-Test
- 13/13 Host-Tests erfolgreich

## Neu in v0.14.0

- gemeinsame `bc2_device_flow`-Schicht im plattformunabhängigen C17-Core
- zentrale Zuordnung von Gerätezuständen zu E-Paper-Bildschirmen
- zentrale Übersetzung physischer Tasten in Geräteereignisse
- ESP32-Einstiegspunkt verwendet Zustandsmaschine, Flow-Mapping und HAL gemeinsam
- Qt-Controller kann dieselben Hardware-Tastenereignisse verarbeiten
- neuer plattformübergreifender Flow-Test
- 12/12 Host-Tests erfolgreich
- Wi-Fi, Bluetooth, Signierung und echte Seed-Verarbeitung bleiben deaktiviert

## Neu in v0.9.1

- Repository-Cleanup ohne neue Features
- Trennung von Qt-Seitenaufbau und MainWindow-Verhaltenslogik
- kleinere und leichter lesbare Hauptdatei
- verbindlicher Development Master Plan
- Sprint-Report mit Build- und Testnachweis
- 11/11 Host-Tests erfolgreich

## Neu in v0.8.0

- echte Electrum-Abfragen für öffentliche BC2-Scripthashes
- bestätigte und unbestätigte Kontostände
- UTXO-Abfrage über `blockchain.scripthash.listunspent`
- Transaktionsverlauf über `blockchain.scripthash.get_history`
- Watch-only-Seite mit Fortschritt, Adresstabelle und Summen
- Empfangs- und Wechseladressen werden getrennt angezeigt
- begrenzter JSON-Empfangspuffer von 4 MiB
- Zuordnung jeder Antwort zu Anfrage, Methode und Scripthash
- Fehler einzelner Anfragen werden nicht als erfolgreicher Vollsync ausgegeben
- Electrum-Clientkennung auf Version 0.5.0 aktualisiert

## Standardnetzwerk

- Server: `infra1.bitcoin-ii.org`
- Port: `50009`
- SSL/TLS: aktiviert; Zertifikatsfehler werden abgelehnt
- Explorer: `https://explorer.bitcoin-ii.org`

## Sicherheitsstatus

Entwicklungssoftware — nicht für echte Guthaben freigegeben.

- Keine echten Seeds, privaten Schlüssel oder PINs eingeben.
- Der Simulator verwendet ausschließlich einen veröffentlichten BIP39-Testvektor.
- Electrum erhält nur öffentliche Scripthashes.
- Explorer-Aufrufe enthalten ausschließlich öffentliche Adressen.
- Watch-only-Daten sind nicht vertrauenswürdig genug für eine Signaturfreigabe.
- PSBT-Signierung und produktive Hardware-Freigabe bleiben deaktiviert.
- Eine einzelne Electrum-Quelle bietet noch keine unabhängige Serververifikation.

## Bauen und testen

### Wallet-Core

Voraussetzungen: CMake 3.20+, C17-Compiler und OpenSSL 3.

```bash
./scripts/build-core.sh
```

### Wallet-Core und Qt-Simulator

Zusätzlich erforderlich: Qt 6.4+ mit Widgets und Network.

```bash
./scripts/build-all.sh
```

Starten:

```bash
./build/simulator/bc2-wallet-simulator
```

Unter Windows:

```text
scripts\build-all.bat
build\simulator\Release\bc2-wallet-simulator.exe
```

## Bedienung der Synchronisation

1. Simulator starten.
2. Unter „Netzwerk“ die SSL-Verbindung kontrollieren.
3. „Watch-only“ öffnen.
4. „Jetzt synchronisieren“ drücken.
5. Der Simulator fragt für 10 Empfangs- und 5 Wechseladressen Kontostand, Verlauf und UTXOs ab.

Diese Version verwendet noch einen festen, veröffentlichten Testvektor. Der spätere Import eines echten Watch-only-Kontokontexts muss ohne Seed und ohne private Schlüssel erfolgen.

## Dokumentation

- `docs/WATCH-ONLY-SYNC-PLAN.md` – verbindlicher Stand und nächste Ausbaustufen
- `docs/PHASES-3-6-PLAN.md` – Gesamtplan für Phasen 3–6
- `docs/SECURITY.md` – Sicherheitsgrenzen und offene Risiken
- `docs/DEVELOPMENT-MASTER-PLAN.md` – verbindliche Roadmap und Entwicklungsrichtlinie
- `docs/SPRINT-01-REPORT.md` – Abschlussbericht für v0.9.1
- `docs/QT-FOUNDATION.md` – Architektur des modernen Qt-Grundgerüsts
- `docs/SPRINT-02-REPORT.md` – Abschlussbericht für v0.10.0
- `docs/SPRINT-03-DEVELOPMENT-PLAN.md` – verbindlicher Plan für das Komponentensystem
- `docs/SPRINT-03-REPORT.md` – Abschlussbericht für v0.11.0
- `docs/SPRINT-04-DEVELOPMENT-PLAN.md` – verbindlicher Plan für vollständige Wallet-Seiten
- `docs/SPRINT-04-REPORT.md` – Abschlussbericht für v0.12.0
- `docs/SPRINT-05-DEVELOPMENT-PLAN.md` – verbindlicher Plan für Navigation und Design Polish
- `docs/SPRINT-05-REPORT.md` – Abschlussbericht für v0.13.0
- `docs/SPRINT-06-DEVELOPMENT-PLAN.md` – verbindlicher Plan für HAL- und Gerätefluss-Synchronisierung
- `docs/SPRINT-06-REPORT.md` – Abschlussbericht für v0.14.0
- `docs/SPRINT-07-DEVELOPMENT-PLAN.md` – verbindlicher Plan für Waveshare-Bring-up
- `docs/SPRINT-07-REPORT.md` – Abschlussbericht für v0.16.0
- `TEST-RESULTS.txt` – tatsächlich ausgeführte Prüfungen

## v0.8.0 – Geräteabläufe und Sperrlogik

- gemeinsame C17-Zustandsmaschine für Simulator und spätere Hardware
- Sperrbildschirm und explizite Entsperrung im Qt-Simulator
- fünf Fehlversuche mit zeitgesteuerter, steigender Sperrphase
- automatische Sperre nach fünf Minuten Inaktivität
- definierte Bestätigungs- und Abbruchpfade für Empfang, Transaktion und Einstellungen
- Test-PIN im Simulator: `2468` – ausschließlich Entwicklungswert, niemals echte PIN verwenden

Der vollständige Plan steht in `docs/DEVICE-FLOWS-AND-LOCKING-PLAN.md`.


## v0.8.0 PSBT-Transaktionsprüfung
Unterstützt PSBT v0 mit nativen P2WPKH-Inputs über witness_utxo. Zeigt Inputs, Outputs, exakte Gebühr und gegen Watch-only-Scripts geprüftes Wechselgeld. Signieren bleibt deaktiviert.


## Hardware-Abstraktion v0.8.0

Der gemeinsame C17-Core besitzt jetzt eine HAL für E-Paper, Tasten, Zeit, Zufall, nichtflüchtigen Speicher und USB. Siehe `docs/HARDWARE-ABSTRACTION-PLAN.md`.

## ESP32-S3 Hardware (v0.9.0)
Zielboard: Waveshare ESP32-S3-ePaper-1.54 (200×200). Für den Hardware-Build wird ESP-IDF 5.5.0 oder neuer benötigt:

```bash
./scripts/build-hardware.sh
```

Vor Aktivierung des physischen Displays muss die Boardrevision V1 oder V2 bestätigt werden; Waveshare kennzeichnet die Beispielprogramme als nicht austauschbar.


## Desktop readiness v0.16.0

Die Desktop-App speichert Theme, Fenstergeometrie und Electrum-Verbindungsdaten lokal. Seeds, private Schlüssel, PINs und Signierdaten werden niemals als Desktop-Einstellung gespeichert. Der reale Waveshare-Wire-up folgt erst nach Ankunft der Hardware.

## Desktop-Builds

Die verbindliche Anleitung für Linux, Windows und macOS liegt unter:

```text
docs/DESKTOP-BUILD-GUIDE.md
```

Unter Linux kann der Desktop-Build mit `./scripts/build-desktop-linux.sh` gestartet werden. Native Windows- und macOS-Pakete werden reproduzierbar über `.github/workflows/desktop-release.yml` erzeugt.
