# Verbindlicher Entwicklungsplan – BC2 Cold Wallet

Stand: Version 0.2.0

## Zielbild

Der Wallet-Core ist eine plattformunabhängige C17-Bibliothek. Der Qt-6-Simulator und die spätere Hardware-Firmware verwenden dieselben öffentlichen Core-Schnittstellen. UI, USB, Display, PIN-Eingabe und sichere Speicherung liegen außerhalb des Core und dürfen kryptografische Geheimnisse nur über klar definierte Schnittstellen verwenden.

## Punkt 1 – Wallet-Core

### Phase 1A – Fundament und stabile API

Status: begonnen, teilweise umgesetzt.

- C17-Bibliothek mit strikten Compilerwarnungen.
- Hashes, HMAC, PBKDF2, BIP39, BIP32, secp256k1, Base58Check, Bech32 und ECDSA.
- Zentrale BC2-Netzwerkparameter.
- Wallet-Service für BIP84-Empfangsadressen.
- Deterministische Standard-Testvektoren.
- Sicheres Löschen temporärer Geheimnisse.

Abnahmekriterium: Build und alle Unit-Tests laufen ohne Fehler.

### Phase 1B – Schlüssel- und Wallet-Zustandsmodell

- Wallet-Kontext ohne globale Geheimnisse.
- Seed-Erzeugung, Import und kontrollierte Lebensdauer.
- Watch-only-Datenmodell mit xpub statt privater Schlüssel.
- Adressindex- und Kontenverwaltung.
- Fehlercodes und dokumentierte API-Verträge.
- Austauschbare Kryptografie- und Entropie-Backends für Desktop und Hardware.

Abnahmekriterium: Geheimnisse sind im Core eindeutig abgegrenzt und alle Lebenszyklen durch Tests abgedeckt.

### Phase 1C – Transaktionsprüfung

- Bitcoin-kompatibler Transaktionsparser.
- PSBT-Parser mit strikten Größen- und Formatgrenzen.
- Ermittlung von Inputs, Outputs, Gebühren und Wechselgeld.
- Policy Engine für erlaubte Script-Typen und Ableitungspfade.
- Vollständiges, UI-neutrales Bestätigungsmodell.
- Signieren erst nach expliziter Freigabe durch die Plattformschicht.

Abnahmekriterium: veröffentlichte Bitcoin-Testvektoren sowie negative und manipulierte Fälle werden vollständig getestet.

### Phase 1D – Härtung

- Wechsel von veralteten OpenSSL-APIs auf eine langfristig unterstützte Schnittstelle oder dokumentierte geprüfte secp256k1-Bibliothek.
- Fuzzing für Parser, Pfade und Kodierungen.
- Sanitizer-Builds und statische Analyse.
- Reproduzierbare Builds.
- Unabhängige Kryptografie- und Sicherheitsprüfung.

Abnahmekriterium: kein Produktionsreife-Status ohne externes Audit und Hardwaretests.

## Punkt 2 – Qt-6-Simulator

### Phase 2A – UI-Grundgerüst

Status: in Version 0.2.0 umgesetzt.

- Modernes, dunkles Wallet-Dashboard.
- Empfangsablauf mit Adresse, Ableitungspfad und Bestätigung.
- Direkte Nutzung des C17-Wallet-Cores.
- Nur veröffentlichter Testvektor; keine echten Seeds oder PINs.
- Deutliche Kennzeichnung von Simulatoraktionen.

Abnahmekriterium: Qt-6-Build startet und Empfangsadressen werden über den Core erzeugt.

### Phase 2B – Gerätezustände und Navigation

- Setup-, Locked-, Unlock-, Dashboard-, Receive- und Settings-Zustände.
- Simulierte Gerätebuttons und E-Paper-Auflösung.
- PIN-Eingabe nur als Testzustand, ohne produktive Sicherheitsbehauptung.
- Geräteereignisse über eine abstrakte Transport-Schnittstelle.
- UI-Komponententests.

### Phase 2C – Transaktionsfluss

- Import einer Test-PSBT.
- Schrittweise Darstellung aller sicherheitsrelevanten Details.
- Bestätigen oder Ablehnen im simulierten Gerät.
- Signaturergebnis nur nach Core-Policy-Freigabe.
- Manipulations- und Fehleransichten.

### Phase 2D – Hardware-nahe Simulation

- E-Paper-Farb- und Refresh-Beschränkungen.
- Exakte Displaymaße und Buttonbelegung der Zielhardware.
- USB-Protokoll-Simulator.
- Gleiche Zustandsmaschine für Desktop und Firmware.

## Reihenfolge

1. Wallet-Core API und Zustandsmodell stabilisieren.
2. Simulator-Navigation und Gerätezustände ausbauen.
3. PSBT-Parser und Transaktionsprüfung implementieren.
4. Transaktions-UI vollständig anbinden.
5. Hardware-Abstraktionsschicht und konkrete Hardware auswählen.
6. Firmware-Portierung und Vorserientests.

Diese Reihenfolge bleibt verbindlich, solange kein nachgewiesener technischer oder sicherheitsrelevanter Grund eine Änderung erfordert.
