# Sprint 7 – Abschlussbericht

## Ergebnis

Sprint 7 wurde als sichere Hardware-Bring-up-Version `v0.15.0` umgesetzt.

## KISS-Prüfung des Ausgangsstands

Der gemeinsame Core war modular und testbar. Der Waveshare-BSP bestand jedoch überwiegend aus stark verdichteten Einzeilern. Dadurch waren Fehlerpfade, Argumentprüfungen und Hardwaregrenzen schwer lesbar. Der BSP wurde deshalb ohne Architekturwechsel in kleine, sprechend benannte Funktionen zerlegt.

## Umgesetzt

- neuer gemeinsamer C17-Geräteservice `bc2_device_service`
- sichere USB-Kommandos `PING`, `GET_INFO` und `GET_STATE`
- Antwortframes verwenden das bestehende BC2-USB-Protokoll
- USB Serial/JTAG im ESP32-S3-BSP als HAL-Transport angebunden
- USB-Protokoll und Logausgabe werden getrennt gehalten
- NVS-, RNG-, Zeit-, USB- und Fehlerpfade lesbar strukturiert
- explizite Bereitschaftsabfragen für Display und Buttons
- Firmware-Einstiegspunkt in kleinere Aufgaben zerlegt
- neuer Host-Test für den Geräteservice
- Teststand von 12 auf 13 Tests erweitert

## Sicherheitsentscheidung

Die konkrete Waveshare-Boardrevision und damit die verbindliche Pinbelegung liegen im Repository weiterhin nicht bestätigt vor. Daher wurden keine GPIO-Nummern erfunden. Displayausgaben werden im Bring-up-Modus protokolliert; Buttons melden weiterhin `UNAVAILABLE`. Diese Sperre ist beabsichtigt und verhindert Schäden oder falsche Hardwareannahmen.

## Buildstatus

- Host-CMake: erfolgreich
- C17-Wallet-Core: erfolgreich
- CLI: erfolgreich
- Warnungen als Fehler: erfolgreich
- Host-Tests: 13/13 bestanden
- Qt: in dieser Umgebung nicht verfügbar
- ESP-IDF: `idf.py` in dieser Umgebung nicht installiert

## Nächster Schritt

Sprint 8 beginnt erst nach dem Hardware-Bring-up auf dem echten Waveshare-Board. Dafür werden die exakte Boardrevision beziehungsweise die Hersteller-Pinbelegung und ein realer Flash-Test benötigt. Danach folgen physisches E-Paper, Buttons und Hardware-UI.
