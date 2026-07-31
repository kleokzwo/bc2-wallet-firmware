# Sprint v0.18.0 – Watch-Only Wallet

## Ziel

Der Qt-Desktop-Simulator soll als praktisch nutzbare BC2-Watch-only-Wallet auftreten, ohne Seeds, private Schlüssel oder Signierfunktionen zu speichern oder zu verwenden.

## Implementiert

### Electrum-Verbindung

- Manuelles Verbinden und Trennen
- SSL/TLS mit strikter Zertifikatsprüfung
- Request-Timeout von 15 Sekunden
- Auto-Reconnect mit Backoff bis maximal 30 Sekunden
- Statusmeldungen für Verbinden, Online, Offline und Fehler
- Header-Abonnement zur Anzeige der aktuellen Blockhöhe

### Synchronisation

- Manueller Refresh
- Automatischer Refresh alle fünf Minuten
- Fortschrittsanzeige
- Letzte erfolgreiche Synchronisation mit Datum und Uhrzeit
- Fehlerhafte Einzelabfragen führen nicht zu einem falschen Vollerfolg

### Wallet-Anzeige

- Gesamtguthaben
- Bestätigtes Guthaben
- Unbestätigtes Guthaben
- Anzahl Transaktionen und UTXOs
- Adresstabelle für Empfang und Wechselgeld
- Verlauf mit TXID, Status, bekanntem Eingangsbetrag und Bestätigungen

Hinweis: Electrum `get_history` liefert allein keinen Zeitstempel und keinen vollständigen Netto-Betrag. Datum/Uhrzeit bleiben daher bewusst als nicht verfügbar markiert, statt Werte zu erfinden. Eine spätere Erweiterung kann Transaktions- und Blockdaten zusätzlich abrufen.

### Erweiterbarkeit

- `WatchAccount` bereitet mehrere Konten vor.
- Das Account-Modell besitzt ein XPUB-Feld für einen späteren sicheren Watch-only-Import.
- Es wird weiterhin nur ein Demo-Konto aufgebaut; keine vollständige Multi-Account-Bedienoberfläche wurde vorgezogen.

## Sicherheit

- Keine Seed-Speicherung
- Keine Private-Key-Speicherung
- Keine Signierung
- Netzwerkcode bleibt außerhalb des C17-Wallet-Cores im Qt-Simulator
- Der Wallet-Core bleibt unabhängig von Qt und ESP32

## Prüfung

- C17-Core erfolgreich gebaut
- 13/13 Host-Tests erfolgreich
- Qt-Desktop-Build konnte in der Ausführungsumgebung nicht durchgeführt werden, da Qt 6 nicht installiert ist
- Der Qt-Code wurde strukturell und statisch geprüft

## Nächster Sprint

v0.19.0 – Receive:

- QR-Code
- Adresse kopieren
- Verify on Device
- Address Labels
- Gap Limit
