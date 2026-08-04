# BC2 Cold Wallet

## Version 0.29.7

Hotfix: PIN- und Bestätigungstasten funktionieren wieder zuverlässig, weil
der USB-Empfang den einzigen Firmware-Task nicht mehr blockiert. Die physische
Eingabe wird in jedem Loop vor USB verarbeitet. Ein bestätigtes oder
abgelehntes Ergebnis bleibt auf dem Gerät gespeichert und
wird bei wiederholten USB-Abfragen erneut geliefert. Erst eine neue, vollständig
validierte Transaktionsanfrage ersetzt das vorherige Ergebnis.

Die erste USB-Antwort bedeutet nur, dass das Gerät die Transaktionsprüfung
angenommen hat. Erfolg wird erst nach Geräte-PIN, vollständiger Anzeige und
physischer Bestätigung mit beiden Tasten gemeldet. Die einzelne BOOT-Taste
lehnt die Transaktion ab. Ein USB-Verbindungsabbruch bleibt ein eigener,
sicherer Fehlerzustand. Während einer laufenden Prüfung wird keine zweite
Transaktionsanfrage angenommen. Langsame E-Paper-Aktualisierungen können die
Abfrage verzögern, aber weder Tastendrücke dauerhaft verdrängen noch die
abschließende Entscheidung vernichten.

Sicherheitsgrenze: Es wird weiterhin nicht signiert und nichts gesendet.
v0.29.7 überträgt weiterhin nur die vom Desktop geprüfte Zusammenfassung;
die spätere Signaturstufe darf erst beginnen, wenn die Hardware die vollständige
PSBT selbst verarbeitet und ihre eigene Wallet-Policy besitzt.

## Version 0.28.0

v0.28.0 verbindet die lokal geprüfte PSBT-Transaktionsansicht mit der echten
Waveshare-Hardware. Eine Transaktion mit genau einem externen BC2-Empfänger kann
aus dem PSBT-Dialog an das Gerät übertragen werden. Das Gerät validiert das
begrenzte Nachrichtenformat, verlangt den Geräte-PIN erneut und zeigt Empfänger,
Betrag, Gebühr und verifiziertes Wechselgeld vor der physischen Bestätigung an.

Sicherheitsgrenze: Diese Version erzeugt keine Signatur und sendet keinen
Private Key, Seed oder PIN über USB. Die Hardware erhält in v0.28.0 eine vom
Desktop geprüfte Zusammenfassung, noch nicht die vollständige PSBT. Daher darf
die Bestätigung nicht als Freigabe für eine spätere Signierung verwendet werden.
Mehrere externe Empfänger sowie unvollständige Gebühren- oder
Wechselgeldinformationen werden abgelehnt.

Für den gefahrlosen Hardwaretest liegt unter `test-data/` die deterministische,
unsigned Datei `bc2-safe-test-transaction.psbt`. Sie enthält ausschließlich
erfundene Daten, kein Wechselgeld und keine privaten Schlüssel. Die erwarteten
Beträge stehen in `test-data/README.md`.

## Version 0.27.2

v0.27.2 verbindet den Empfangen-Flow des Qt-Simulators direkt mit der
hardwarebestätigten USB-Schnittstelle. Der Simulator erkennt das BC2-Gerät
automatisch und überträgt die angezeigte öffentliche Adresse. Die Hardware
validiert die Adresse und fordert anschließend den Geräte-PIN an. Serielle
Monitore müssen vor der Verwendung geschlossen werden.

Der Desktop-Build benötigt zusätzlich das Qt-6-Modul SerialPort.

## Verifizierter Stand v0.27.2

- Desktop-Build auf Kali Linux erfolgreich.
- 20 Hosttests vorhanden; der zuvor fest codierte v0.27.0-Vergleich verwendet
  jetzt die zentrale Firmware-Version.
- Automatische Erkennung ueber `/dev/ttyACM0` auf echter Hardware bestaetigt.
- Ablauf Desktop -> USB -> Geraete-PIN -> Adressanzeige -> physische Bestaetigung
  auf dem Waveshare-Board bestaetigt.
- Der Display-Refresh wurde verbessert. Das E-Paper bleibt bauartbedingt langsam;
  sichtbares Rest-Ghosting muss bei jeder neuen Firmware weiterhin auf echter
  Hardware geprueft werden.

Diese Version ist weiterhin Entwicklungssoftware. Keine echten Seeds oder
Guthaben verwenden; Secure Boot, Flash Encryption, Security Review und der
vollstaendige Signaturablauf fehlen noch.

## Version 0.26.0

v0.26.0 verbindet erstmals eine echte BC2-Mainnet-Empfangsadresse mit der
Hardware-PIN-Freigabe. Das Desktop-Hilfsprogramm uebertraegt nur die oeffentliche
Adresse; Seed, privater Schluessel und PIN werden niemals ueber USB gesendet.

Hardwaretest nach dem Entsperren:

```bash
python tools/bc2_device_probe.py --port /dev/ttyACM0 \
  --receive-address bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4
```

Unter Windows beispielsweise `--port COM5` verwenden. Danach fordert das
Geraet den sechsstelligen PIN erneut an und zeigt die vollstaendige Adresse.
Die Beispieladresse dient ausschliesslich dem Test und darf nicht fuer echte
Zahlungen verwendet werden.

Unterstuetzt wird in diesem Sprint BC2 Mainnet Native SegWit (P2WPKH, `bc1q`).
Andere Netzwerke, fehlerhafte Pruefsummen und Grossschreibung werden abgelehnt.

## Version 0.25.2

Hotfix v0.25.2 verhindert einen Task-Watchdog-Reset waehrend der
PIN-Ableitung. PBKDF2-HMAC-SHA-256 bleibt bei 100.000 Runden, gibt auf dem
ESP32-S3 aber regelmaessig Rechenzeit an FreeRTOS zurueck.

Hotfix v0.25.1 behebt den auf echter Hardware festgestellten Stackueberlauf
des ESP-IDF-Main-Tasks. Die BC2-Anwendung verwendet jetzt einen eigenen
FreeRTOS-Task mit 16 KiB Stack. Die Funktionen aus v0.25.0 bleiben unveraendert.

Sprint v0.25.0 ersetzt den fest eingebauten Test-PIN durch die erste persistente
Geräte-PIN-Grundlage auf echter Hardware.

## Neu in v0.25.0

- Benutzer legt beim ersten Entsperren einen sechsstelligen PIN an und wiederholt ihn.
- Kein Klartext-PIN und kein fester Entwicklungs-PIN in der Firmware.
- PBKDF2-HMAC-SHA-256 mit 100.000 Runden und 128-Bit-Zufallssalt.
- Fehlversuche werden in NVS gespeichert; ab dem dritten Fehler steigt die Wartezeit.
- Nach zehn Fehlversuchen gilt eine Stunde Sperrzeit, auch nach einem Neustart erneut.
- Zentrale Policy: Geräte-PIN fuer Entsperren, Empfangsadressen und Transaktionen;
  separater Root-PIN fuer das spaetere Erstellen neuer Wallets.

Die Bedienung bleibt: oben/rechts, unten/links, beide Tasten/bestaetigen und `[<]`/loeschen.

## Hardware-Bedienung

1. Auf dem Sperrbildschirm beide Tasten gemeinsam drücken.
2. Mit PWR nach rechts oder mit BOOT nach links navigieren.
3. Beide Tasten gemeinsam drücken, um die markierte Ziffer zu übernehmen.
4. `[<]` markieren und beide Tasten drücken, um die letzte Ziffer zu löschen.

## Sicherheitsgrenze v0.25.0

Die PIN-Speicherung und Rate-Limits sind implementiert. Die eigentlichen Receive-,
PSBT- und Wallet-Erstellungsablaeufe werden in den folgenden Sprints an die zentrale
Freigabe-Policy angebunden. Der Root-PIN ist daher bewusst noch nicht eingerichtet.
Vor einem produktiven Einsatz fehlen weiterhin Security Review, Flash Encryption,
Secure Boot und der vollstaendige Seed-/Signaturablauf.

Die stabilen Waveshare-Displaykomponenten, Refresh-Sequenzen, BUSY-Behandlung und E-Paper-Pins wurden nicht verändert. Nur das bestehende Board-Eingabemodul wurde um die offiziell dokumentierte PWR/BAT_KEY-Leitung auf GPIO 18 erweitert.

## Danach: verbindliche PIN-Freigaben

- Transaktionen benötigen die Geräte-PIN und die physische Gerätebestätigung.
- Neu erzeugte Empfangsadressen benötigen die Geräte-PIN, bevor sie freigegeben werden.
- Das Erstellen einer neuen Wallet benötigt die Root-PIN.

Die zentrale Regel ist implementiert; die jeweiligen Funktionsablaeufe werden in den
folgenden Sprints damit verbunden.

## Grundlage aus v0.23.0

## Neu in v0.23.0

- professioneller BC2-Sperrbildschirm mit Statusleiste, Schloss, USB- und Batterieanzeige
- zentrale BC2-Theme- und Zeichenfunktionen oberhalb des Waveshare-Treibers
- Navigation Controller für entprellte Kurz- und Langdruck-Ereignisse
- kurzer BOOT-Druck startet den Entsperrablauf
- langer BOOT-Druck sperrt einen entsperrten Gerätezustand
- Full- und Partial-Refresh werden anhand des bestehenden Frame-Flags ausgewählt
- zusätzlicher Host-Test für die Navigation

## Grundlage aus v0.22.0

Sprint v0.22.0 integriert den nachweislich funktionierenden offiziellen Waveshare-V2-E-Paper-Unterbau in die BC2-Firmware. Die BC2-Anwendung bleibt über die bestehende HAL vom Boardtreiber getrennt.

## Neu in v0.22.0

- BUSY-Pin aktiv HIGH
- echte Refresh-Zustandsprüfung
- Vollrefresh mit Update-Control `0xF7`
- Diagnosezeiten für BUSY-Aktivierung und Freigabe
- keine falsche Erfolgsmeldung nach 10–20 ms

## Hardware bauen und flashen

```bash
source ~/esp/esp-idf/export.sh
./scripts/build-hardware.sh
cd hardware/esp32s3_waveshare
idf.py -p /dev/ttyACM0 flash monitor
```

Nach `Calling app_main()` werden unter anderem folgende Zeilen erwartet:

```text
Waveshare V2 e-paper initialized: BUSY=21 RST=11 DC=13 CS=12 CLK=10 MOSI=8
Starting full refresh (5000 bytes)
BUSY before activation: 0
BUSY entered active HIGH state after ... ms
BUSY released after ... ms
Full refresh completed
```

Sprint v0.20.0 ergänzt die erste vollständige Send-Vorbereitung: Empfänger, Betrag und Gebührenrate werden geprüft, synchronisierte Watch-only-UTXOs ausgewählt und als unsigned PSBT exportiert. Es findet keine Signierung und kein Broadcast statt.

## Neu in v0.20.0

- BC2-Mainnet-Adressprüfung für SegWit v0 und P2PKH
- deterministische Coin Selection aus synchronisierten Watch-only-UTXOs
- Gebührenberechnung über frei wählbare sat/vB-Rate
- Dust- und Wechselgeldbehandlung
- unsigned PSBT-v0-Erzeugung inklusive `witness_utxo`
- Send-Maske für Empfänger, Betrag und Gebührenrate
- Vorschau von Eingängen, Betrag, Gebühr, Wechselgeld und geschätzten vBytes
- Export als binäre `.psbt`-Datei
- vorhandener PSBT-Prüfdialog bleibt verfügbar
- neuer End-to-End-Transaktionstest
- 15/15 Host-Tests erfolgreich

## Weiterhin enthalten aus v0.19.0

- scanbarer lokaler Receive-QR-Code
- Empfangsadresse, Copy, Explorer und lokale Labels
- konfigurierbares Gap-Limit
- vorbereitete Geräteverifikation

## Neu in v0.18.0

- manuelles Verbinden und Trennen des Electrum-Servers
- SSL/TLS-Verbindung mit strikt abgelehnten Zertifikatsfehlern
- 15-Sekunden-Zeitlimit pro Electrum-Anfrage
- automatische Wiederverbindung mit begrenztem Backoff
- Electrum-Header-Abonnement und aktuelle Blockhöhe
- manueller Refresh und automatischer Refresh alle fünf Minuten
- letzte erfolgreiche Synchronisation mit Datum und Uhrzeit
- Gesamt-, bestätigtes und unbestätigtes Guthaben
- Verlaufstabelle mit TXID, Status, bekanntem Betrag und Bestätigungen
- vorbereitete Account- und XPUB-Strukturen ohne Seed- oder Private-Key-Speicherung
- keine Signierung im Watch-only-Pfad
- 13/13 Wallet-Core-Tests erfolgreich

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

Die aktuelle Entwicklungsversion verwendet noch einen festen, veröffentlichten Testvektor. Der spätere Import eines echten Watch-only-Kontokontexts muss ohne Seed und ohne private Schlüssel erfolgen.

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

### Windows executable name

The packaged Windows application is named `BC2-Cold-Wallet.exe`. The hyphenated file name is intentional: Qt's deployment helper must receive the executable path as one argument during CPack packaging.

## Hardware-Probe v0.22.0

Eine microSD-Karte wird nicht benötigt. Nach dem Flashen kann das Gerät ausschließlich mit öffentlichen Diagnosebefehlen geprüft werden:

```bash
python -m pip install pyserial
python tools/bc2_device_probe.py --list
python tools/bc2_device_probe.py --port /dev/ttyACM0
```

Unter Windows ist der Port typischerweise `COM3`, `COM4` oder ähnlich. Das Probeprogramm sendet keine Seeds, privaten Schlüssel oder PINs.
