# Sprint 6 – Verbindlicher Entwicklungsplan

## Zielversion

BC2 Cold Wallet v0.14.0

## Sprintziel

Die Hardware-Abstraktion wird geprüft und die Geräteabläufe von Qt-Simulator und ESP32 werden über eine gemeinsame, plattformunabhängige C17-Schicht synchronisiert.

## Verbindlicher Umfang

1. Bestehende HAL-Schnittstelle auf Vollständigkeit und Plattformgrenzen prüfen.
2. Eine gemeinsame Zuordnung von Gerätezuständen zu E-Paper-Bildschirmen schaffen.
3. Physische Tastenereignisse zentral in Geräteereignisse übersetzen.
4. ESP32-Referenzfirmware an die gemeinsame Zustandsmaschine und Flow-Schicht anbinden.
5. Qt-Controller für dieselben Hardware-Tastenereignisse vorbereiten.
6. Plattformübergreifende Flow-Tests ergänzen.
7. Build, Tests, README, Changelog und Sprint-Report aktualisieren.

## Nicht Bestandteil

- keine Aktivierung unbestätigter Display- oder Button-GPIOs
- keine Wi-Fi- oder Bluetooth-Aktivierung
- keine Seed-Erzeugung oder Seed-Eingabe
- keine produktive PIN-Prüfung
- keine Transaktionssignierung
- kein produktives USB-Transportprotokoll

## Definition of Done

- gemeinsame C17-Gerätefluss-Schicht vorhanden
- Qt und ESP32 können dasselbe Button-Mapping verwenden
- ESP32-App verwendet die gemeinsame Zustandsmaschine
- Host-Projekt kompiliert mit Warnungen als Fehler
- alle Host-Tests bestehen
- Hardware- und Qt-Toolchain-Einschränkungen sind dokumentiert
- vollständige v0.14.0-ZIP und SHA-256-Prüfsumme erstellt
