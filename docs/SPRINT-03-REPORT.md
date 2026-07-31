# Sprint 3 Report – Wiederverwendbare UI-Komponenten

## Version

`BC2 Cold Wallet v0.11.0`

## Ergebnis

Sprint 3 wurde auf Basis der vollständigen v0.10.0 umgesetzt. Der Sprint führt ein klar getrenntes Qt-Komponentensystem ein und ändert keine Wallet-, Kryptografie-, Netzwerk- oder Hardwarefunktion.

## Umgesetzte Komponenten

- `Bc2Button`: Primary, Secondary, Navigation und Danger
- `Bc2Card`: einheitlicher Oberflächencontainer
- `Bc2Header`: Seitentitel und optionale Beschreibung
- `Bc2Dialog`: gemeinsame modale Dialogbasis
- `Bc2StatusBar`: Neutral, Success, Warning und Error
- `Bc2LoadingWidget`: Fortschritt und unbestimmter Busy-Zustand
- `Bc2QrWidget`: Payload- und Layout-Schnittstelle für die Receive-Seite

## Refactoring

- lokale Card- und Button-Fabriken entfernt
- lokale große Überschriften durch `Bc2Header` ersetzt
- bestehende Seiten auf zentrale UI-Komponenten migriert
- neue Komponenten in `simulator/src/components` gebündelt
- Theme-Regeln um Seitentitel, Abschnittstitel, Statuszustände und Danger-Buttons ergänzt

## Sicherheitsentscheidung QR

Sprint 3 enthält bewusst keinen improvisierten oder ungetesteten QR-Encoder. `Bc2QrWidget` definiert bereits Payload, Darstellungsschnittstelle und Mindestgröße. Die scanbare QR-Erzeugung wird in Sprint 4 zusammen mit der vollständigen Receive-Seite integriert und mit bekannten Testvektoren geprüft.

## Build und Tests

- C17-Wallet-Core: erfolgreich
- CLI: erfolgreich
- Tests: 11 von 11 bestanden
- Qt-Konfiguration: ausgeführt, aber nicht möglich, weil Qt 6.4+ in der Arbeitsumgebung nicht installiert ist
- ESP-IDF-Build: nicht Teil dieses Sprints und Toolchain nicht installiert

## Definition of Done

- [x] Sprintziel umgesetzt
- [x] keine vorgezogenen Wallet-Funktionen
- [x] bestehende Core-Tests grün
- [x] Komponenten klar getrennt
- [x] Duplikate für Card/Button/Header reduziert
- [x] README aktualisiert
- [x] Changelog aktualisiert
- [x] Entwicklungsplan dokumentiert
- [x] vollständige ZIP vorgesehen
- [ ] Qt-Binärdatei gebaut – externe Toolchain fehlt

## Nächster verbindlicher Sprint

Sprint 4: vollständige Wallet-Seiten. Die neue Komponentenbasis muss dabei verwendet werden; parallele lokale UI-Helfer sind nicht zulässig.
