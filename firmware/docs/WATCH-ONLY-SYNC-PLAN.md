# Verbindlicher Entwicklungsstand: Watch-only-Synchronisation

Stand: Version 0.5.0

## In v0.5.0 umgesetzt

1. Aufbau eines öffentlichen Testkontos mit Empfangs- und Wechseladressen.
2. Ableitung der Electrum-Scripthashes aus P2WPKH-ScriptPubKeys.
3. SSL-gesicherte Electrum-Verbindung mit regulärer Zertifikatsprüfung.
4. JSON-RPC-Anfragen für:
   - `blockchain.scripthash.get_balance`
   - `blockchain.scripthash.get_history`
   - `blockchain.scripthash.listunspent`
5. Zuordnung asynchroner Antworten über eindeutige Request-IDs.
6. Speicherung der öffentlichen Ergebnisse im Watch-only-Modell.
7. Fortschritts- und Fehleranzeige im Qt-Simulator.
8. Summierung bestätigter und unbestätigter Beträge.
9. Deduplizierte Zählung bekannter Transaktions-IDs.
10. Sicherheitslimit für eingehende Electrum-Daten.

## Bewusste Grenzen

- Noch kein Import eines Account-xpub oder BC2-spezifischen Deskriptors.
- Noch keine Gap-Limit-Erkennung über dynamische Adressbereiche.
- Noch keine Header-Verifikation oder Merkle-Proof-Prüfung.
- Noch keine Mehrserver-Konsensprüfung.
- Noch keine lokale persistente Watch-only-Datenbank.
- Noch keine Detailansicht vollständiger Transaktionen.
- Netzwerkdaten dürfen nicht allein eine Signaturfreigabe auslösen.

## Verbindliche nächste Ausbaustufen

### v0.5.1 – Stabilisierung

- automatisierte Qt-Tests mit Fake-Electrum-Server
- Timeouts pro Anfrage
- kontrolliertes Wiederverbinden
- Abbruch laufender Synchronisation ohne veraltete Antworten
- validierte Zahlenbereiche und strengere JSON-Schemas

### v0.5.2 – Watch-only-Kontokontext

- Import ausschließlich öffentlicher Kontodaten
- Account-xpub oder standardisierter Descriptor
- keine Seed- oder Private-Key-Eingabe im Desktop-Simulator
- persistenter, versionsgeprüfter öffentlicher Kontokontext

### v0.5.3 – Gap-Limit und Historie

- dynamische Adresssuche mit dokumentiertem Gap Limit
- paginierte Transaktionsansicht
- UTXO-Detailansicht
- Explorer-Verknüpfung je Transaktion

### spätere Sicherheitsstufe

- Electrum-Header-Abonnement
- lokale Header-Kette
- Merkle-Proof-Verifikation
- optionaler Vergleich mehrerer unabhängiger Server

Erst nach diesen Prüfungen darf die Watch-only-Ansicht als belastbare Quelle für die Vorbereitung einer Transaktion gelten. Die finale Freigabe muss weiterhin vollständig auf der Hardware erfolgen.
