# Sprint 8 – Desktop Readiness and KISS Stabilization

## Ausgangslage

Version 0.15.0 besitzt den gemeinsamen C17-Core, vollständige Desktop-Navigation, Wallet-Seiten, Watch-only-Synchronisation und eine vorbereitete Hardware-Abstraktion. Der reale Waveshare-Wire-up ist ausdrücklich nicht Bestandteil dieses Sprints.

## Verbindliches Ziel

Die Desktop-App wird als stabile Referenzanwendung für den späteren Waveshare ESP32-S3 E-Paper-Wire-up vorbereitet. Der Sprint verbessert ausschließlich Desktop-Verhalten, Wartbarkeit und Testbarkeit.

## Umfang

1. Desktop-Einstellungen zentral kapseln.
2. Theme, Fenstergeometrie und Electrum-Verbindung dauerhaft speichern.
3. Keine Einstellungslogik direkt in UI-Seiten verteilen.
4. Version und Dokumentation auf v0.16.0 aktualisieren.
5. Core, CLI und alle Host-Tests vollständig bauen und ausführen.
6. Hardware-Treiber, GPIOs, Seed-Verarbeitung und Signing nicht verändern.

## KISS-Regeln

- Eine Klasse verwaltet ausschließlich Desktop-Einstellungen.
- UI-Code kennt keine QSettings-Schlüssel.
- Standardwerte stehen an genau einer Stelle.
- Keine neue Framework-Abhängigkeit.
- Keine Hardware-Simulation erfindet reale Pins oder Gerätefähigkeiten.

## Definition of Done

- Host-Projekt kompiliert.
- Alle Host-Tests bestehen.
- Desktop-Einstellungen werden zentral gespeichert und geladen.
- Bestehende Sicherheitsgrenzen bleiben unverändert.
- README, CHANGELOG, Sprint-Report und Testergebnis sind aktualisiert.
- Vollständige ZIP und SHA-256-Prüfsumme liegen vor.
