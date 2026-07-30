# BC2 Cold Wallet

## Version 0.5.0

Version 0.5.0 implementiert die erste echte Watch-only-Electrum-Synchronisation im Qt-6-Desktop-Simulator. Der C17-Wallet-Core bleibt die einzige Wallet- und Kryptografieimplementierung.

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
