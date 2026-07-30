# Verbindlicher Entwicklungsplan – Punkt 2: Qt-6-Desktop-Simulator

Stand: Version 0.3.0

## Unveränderliches Ziel

Der Simulator ist die Desktop-Referenzoberfläche für die spätere BC2-Hardware-Wallet. Er verwendet denselben C17-Wallet-Core wie die Firmware. Der Simulator darf keine zweite Wallet- oder Kryptografieimplementierung enthalten.

## Sicherheitsgrenze

- Netzwerkzugriffe verarbeiten ausschließlich öffentliche Blockchain-Daten.
- Seed und private Schlüssel dürfen nie an Electrum, Explorer oder UI-Logging übergeben werden.
- Empfangsadressen gelten erst nach simulierter Gerätebestätigung als freigegeben.
- Die aktuelle PIN- und Gerätebestätigung ist eine sichtbare Simulation und keine produktive Schutzfunktion.
- Der veröffentlichte BIP39-Testvektor ist ausschließlich für Entwicklung und Tests vorgesehen.

## Phase 2A – Grundgerüst

Status: abgeschlossen.

- Qt-6-Widgets-Anwendung
- dunkles Desktop-Layout
- Dashboard
- Empfangsadressen aus dem C17-Core
- Ableitungspfad und simulierte Bestätigung

## Phase 2B – Netzwerk und moderne Desktop-Struktur

Status: in Version 0.3.0 umgesetzt.

- feste Desktop-Navigation mit Übersicht, Empfangen und Netzwerk
- Electrum-Client über Qt Network
- Standardserver `infra1.bitcoin-ii.org:50009`
- SSL/TLS mit normaler Zertifikatsprüfung; Zertifikatsfehler werden nicht ignoriert
- Electrum-Handshake über `server.version`
- offizieller BC2-Explorer `https://explorer.bitcoin-ii.org`
- Öffnen einer Empfangsadresse im Explorer
- deutlich sichtbarer Entwicklungs- und Sicherheitsstatus

Abnahme:

1. Anwendung startet mit Qt 6.4 oder neuer.
2. C17-Core und Simulator werden gemeinsam gebaut.
3. Empfangsadressen stammen ausschließlich aus `bc2_wallet_core`.
4. Electrum-Verbindung zeigt SSL- und Serverstatus nachvollziehbar an.
5. Keine Geheimnisse werden an Netzwerkklassen übergeben.

## Phase 2C – Watch-only-Synchronisation

Nächste verbindliche Phase.

- Ableitung öffentlicher Empfangs- und Wechseladressen aus einem Watch-only-Kontext
- Umwandlung der Scripts in Electrum-Scripthashes
- Abfrage von Verlauf, UTXOs und Kontostand
- lokale, nicht geheime Wallet-Metadaten
- Synchronisationsfortschritt und Fehlerzustände
- strikte Limits, Timeouts und nachvollziehbare Serverantworten

Abnahme: Ein Testkonto kann ohne Seed- oder Private-Key-Übertragung seinen öffentlichen Kontostand und Verlauf anzeigen.

## Phase 2D – Geräte-Zustandsmaschine

- Setup, Locked, Unlock, Dashboard, Receive, Review und Settings
- separates simuliertes E-Paper-Gerät im Desktopfenster
- physische Button-Ereignisse als abstrakte Geräteeingaben
- PIN nur als nicht produktiver Testzustand
- identische Zustandsmaschine für Simulator und spätere Firmware

## Phase 2E – Transaktionsprüfung

Voraussetzung: Wallet-Core Phase 1C.

- Test-PSBT laden
- Inputs, Ziele, Beträge, Gebühren und Wechselgeld schrittweise anzeigen
- Warnungen bei unbekannten Pfaden oder Scripts
- Bestätigen oder Ablehnen ausschließlich in der Geräteansicht
- Signieraufruf erst nach erfolgreicher Core-Policy-Prüfung

## Phase 2F – Hardware-nahe Referenz

- exakte E-Paper-Auflösung und Farbbeschränkung
- Refresh-Verhalten und Ghosting-Simulation
- finale Buttonbelegung
- USB-Transport-Simulator
- Golden-Screenshot- und Ablauf-Tests

## Architekturregeln

1. `simulator/` enthält UI, Netzwerk und Desktop-Plattformcode.
2. `firmware/` enthält den gemeinsamen C17-Core.
3. Netzwerkklassen erhalten niemals Mnemonic, Seed oder Private Keys.
4. Explorer-Aufrufe erfolgen ausschließlich für öffentliche Adressen oder Transaktions-IDs.
5. SSL-Zertifikatsfehler werden niemals automatisch akzeptiert.
6. Neue Funktionen werden erst nach Build, Tests und Dokumentation als abgeschlossen markiert.
