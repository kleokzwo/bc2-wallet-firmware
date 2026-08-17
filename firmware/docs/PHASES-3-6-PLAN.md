# Verbindlicher Entwicklungsplan – Punkte 3 bis 6

Stand: Version 0.4.0

## Nicht verhandelbare Sicherheitsgrenzen

- Watch-only verarbeitet ausschließlich öffentliche Schlüssel, Adressen, Scripts, Scripthashes und Blockchain-Daten.
- Netzwerkcode erhält niemals Mnemonic, Seed, privaten Schlüssel oder PIN.
- Die Geräte-Zustandsmaschine ist deterministisch und unabhängig von Qt.
- PSBT-Dateien werden vor jeder Anzeige und vor jedem späteren Signieraufruf strikt geparst, begrenzt und gegen Wallet-Policy geprüft.
- Version 0.4.0 signiert keine PSBT. Der neue Prüfer ist ein Strukturprüfer, keine vollständige Transaktionsfreigabe.
- Die E-Paper-Referenz simuliert nur Darstellung und Eingaben. Sie ist noch kein Hardware-Sicherheitsnachweis.

## Punkt 3 – Watch-only-Synchronisation

### 3A: Öffentlicher Kontokontext – begonnen in v0.4.0
- Empfangs- und Wechseladressensätze aus dem gemeinsamen Wallet-Core
- Electrum-Scripthash aus P2WPKH-ScriptPubKey
- ausschließlich öffentliche Metadaten im Qt-Modell
- feste, kleine Demo-Grenzen für Tests

### 3B: Electrum-Abfragen – nächste Version
- `blockchain.scripthash.get_balance`
- `blockchain.scripthash.get_history`
- `blockchain.scripthash.listunspent`
- Request-ID-Zuordnung, Timeout, Antwortgrößenlimit und Abbruch
- Fortschritt pro Adresse und zusammengefasster Kontostand

### 3C: Gap-Limit und Persistenz
- Empfangs- und Wechselzweig getrennt
- Gap-Limit 20 als dokumentierter Standard, konfigurierbar nur innerhalb sicherer Grenzen
- lokale Speicherung ausschließlich nicht geheimer Metadaten
- Reorg-, Offline- und Teilfehler-Zustände

Abnahme: Kontostand, Verlauf und UTXOs eines Testkontos werden ohne Übertragung geheimer Daten reproduzierbar angezeigt.

## Punkt 4 – Geräte-Zustandsmaschine

### 4A: Gemeinsamer C17-Zustandskern – umgesetzt in v0.4.0
- Boot
- Setup erforderlich
- Gesperrt
- Entsperren
- Dashboard
- Empfang prüfen
- Transaktion prüfen
- Einstellungen
- Fehlerzustand
- begrenzte Fehlversuche im Testmodell

### 4B: Eingabe- und Policy-Schicht
- abstrakte Tastenereignisse
- automatische Sperre
- PIN-Eingabe ohne Klartext-Logging
- Trennung zwischen UI-Navigation und Sicherheitsfreigabe

### 4C: Ablauf- und Negativtests
- ungültige Übergänge
- Sperre während Review
- Abbruch und Timeout
- Fehler- und Wiederanlaufpfade

Abnahme: Simulator und spätere Firmware verwenden dieselben Zustandsübergänge und dieselben Tests.

## Punkt 5 – Transaktionsprüfung und PSBT-Anzeige

### 5A: Begrenzter PSBT-Strukturprüfer – umgesetzt in v0.4.0
- PSBT-Magic
- CompactSize-Längenprüfung
- Größenlimit 1 MiB
- Limit globaler Schlüssel/Wert-Paare
- Erkennung der globalen unsigned transaction
- keine Signierung

### 5B: Vollständiger BIP174/BIP370-Parser
- globale, Input- und Output-Maps
- unbekannte Schlüssel kontrolliert erhalten oder ablehnen
- Duplikate, nicht-kanonische Längen und widersprüchliche Felder ablehnen
- Parser-Fuzzing und Korpus negativer Testdateien

### 5C: BC2-Transaktionsmodell
- Inputs, eigene UTXOs und Beträge
- Zieladressen und Beträge
- Wechselgelderkennung über eigene Ableitungspfade
- absolute und relative Gebühr
- Warnungen für unbekannte Scripts, Pfade und ungewöhnliche Gebühren

### 5D: Gerätefreigabe
- vollständige schrittweise Anzeige
- Bestätigen/Ablehnen nur in der Geräteansicht
- Signieraufruf ausschließlich nach erfolgreicher Policy-Prüfung
- Signatur bleibt zunächst Testfunktion und wird separat auditiert

Abnahme: Ein PSBT-Testkorpus wird korrekt angenommen oder mit eindeutigem Fehler abgelehnt; jede freigegebene Information ist auf der Geräteansicht sichtbar.

## Punkt 6 – Hardware-nahe E-Paper-Referenz

### 6A: Pixelnahe Vorschau – begonnen in v0.4.0
- 296 × 128 Referenzfläche
- Schwarz/Weiß-Darstellung
- Titel, Hauptinhalt und Tastenhinweise
- keine Farbcodierung als alleinige Sicherheitsinformation

### 6B: Display-Abstraktion
- framebuffer-unabhängige Renderbefehle
- feste Typografie- und Umbruchregeln
- Voll- und Teilrefresh
- Ghosting- und Refresh-Verzögerungssimulation

### 6C: Hardware-Eingaben und USB-Transport
- linke/rechte/Bestätigen/Zurück-Ereignisse
- Host-Anfragen dürfen Zustände nicht überspringen
- protokollierter USB-Nachrichtenrahmen mit Größen- und Versionsprüfung

### 6D: Golden Tests
- reproduzierbare Screenshots für alle Sicherheitsabläufe
- lange Adressen, hohe Beträge, viele Outputs und Fehlertexte
- Vergleich Simulator gegen Firmware-Framebuffer

Abnahme: Die Desktop-Referenz entspricht Auflösung, Eingaben und Ablauf der ausgewählten Hardware und besitzt reproduzierbare Golden Tests.

## Lieferreihenfolge

1. v0.4.0: gemeinsame Zustandsmaschine, Watch-only-Modell/Scripthashes, begrenzter PSBT-Strukturprüfer, E-Paper-Vorschau.
2. v0.5.0: echte Watch-only-Electrum-Synchronisation mit Kontostand, Verlauf und UTXOs.
3. v0.6.0: vollständige Geräteabläufe, Eingabeabstraktion und Sperrlogik.
4. v0.7.0: vollständiger PSBT-Parser und Transaktionsanzeige, weiterhin ohne Produktionssignierung.
5. v0.8.0: hardware-nahe Display-/USB-Abstraktion und Golden Tests.
6. Danach: Zielhardware, Secure Element/Schlüsselspeicher, Firmware-Portierung und externe Sicherheitsprüfung.
