# Verbindlicher Entwicklungsplan – Sprint 12 / v0.21.0

## Ziel
Die physische Waveshare-ESP32-S3-Hardware sicher und reproduzierbar in Betrieb nehmen. Der erste Hardware-Sprint konzentriert sich auf USB, Geräteidentität, Zufallsquelle und NVS. Display und Tasten werden erst nach bestätigter Board-Revision und Pinbelegung aktiviert.

## Lieferumfang
1. Robuste USB-Serial/JTAG-Kommunikation für fragmentierte und zusammengefasste Frames.
2. Persistenter USB-Stream-Decoder im plattformunabhängigen C17-Bereich.
3. Geräteabfragen für Ping, Firmwareinformation, Zustand und Fähigkeiten.
4. Hardware-Probeprogramm für Windows, Linux und macOS.
5. Wiederhergestellte Build-Skripte für Core, Desktop und ESP-IDF.
6. Dokumentierter Flash- und Probeablauf ohne microSD-Karte.
7. Tests für fragmentierte USB-Daten und Geräteantworten.

## Sicherheitsgrenzen
- keine Seeds oder Private Keys über USB
- keine PIN-Übertragung
- keine Signierung
- kein Display-/Button-GPIO ohne bestätigte Board-Revision
- Wi-Fi und Bluetooth bleiben deaktiviert
- microSD wird weder benötigt noch verwendet

## Definition of Done
- Host-Build ohne Warnungen
- alle Host-Tests erfolgreich
- USB-Stream-Decoder testet Teilframes, mehrere Frames und Stördaten
- ESP-IDF-Projekt bleibt vom Desktop und von Qt getrennt
- vollständige ZIP, SHA-256, README, Changelog und Sprint-Bericht
