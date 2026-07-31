# Sprint 7 – Verbindlicher Entwicklungsplan

## Version

Zielversion: `BC2 Cold Wallet v0.15.0`

## Sprintziel

Die Waveshare-Referenzfirmware erhält eine nachvollziehbare Bring-up-Basis für Flash, USB, Display und Tasten. Es werden nur Hardwarefunktionen aktiviert, deren Verhalten ohne erfundene Board-Pins sicher implementiert werden kann.

## Verbindlicher Umfang

1. bestehenden Waveshare-BSP nach KISS refaktorieren
2. USB Serial/JTAG als binären BC2-Transport anbinden
3. gemeinsamen C17-Geräteservice für sichere USB-Kommandos bereitstellen
4. Firmwareversion und Geräteidentität über USB ausgeben
5. Display- und Button-Bereitschaft explizit melden
6. reale Display- und Button-GPIOs bis zur bestätigten Boardrevision sperren
7. Host-Build und alle Tests ausführen
8. ESP-IDF- und Qt-Build versuchen und Einschränkungen dokumentieren
9. README, Changelog, Testbericht und Sprintbericht aktualisieren
10. vollständige ZIP und SHA-256-Prüfsumme erzeugen

## Erlaubte USB-Kommandos

- `PING`
- `GET_INFO`
- `GET_STATE`

## Ausdrücklich ausgeschlossen

- Seed-Export
- private Schlüssel
- Signierung
- PIN-Übertragung
- Display- oder Button-GPIOs ohne bestätigte Boardrevision
- Wi-Fi und Bluetooth

## Definition of Done

- Core und CLI kompilieren mit Warnungen als Fehler
- alle Host-Tests bestehen
- USB-Geräteservice ist separat getestet
- Hardware-BSP ist lesbar und in kleine Funktionen zerlegt
- keine erfundenen Pinbelegungen
- Dokumentation ist auf v0.15.0 aktualisiert
- vollständige ZIP ist geprüft
