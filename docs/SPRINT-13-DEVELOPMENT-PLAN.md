# Verbindlicher Entwicklungsplan – Sprint 13 / v0.21.1

## Ziel

Aktivierung der physischen Bedienelemente des vorhandenen Waveshare ESP32-S3-ePaper-1.54 V2, ohne Änderungen am plattformunabhängigen C17-Wallet-Core.

## Verbindlicher Umfang

1. V2-Boardprofil fest konfigurieren.
2. 200x200-Monochrom-E-Paper über SPI anbinden.
3. Sicheren Start- und Sperrbildschirm auf dem echten Display ausgeben.
4. BOOT-Taste als vorläufige Bestätigen-Taste mit Entprellung anbinden.
5. USB-Geräteidentität um Revision, Display und Buttons erweitern.
6. Keine Seeds, privaten Schlüssel oder Signierung hinzufügen.
7. Wi-Fi, Bluetooth und microSD weiterhin nicht verwenden.

## Hardwareprofil V2

- EPD power: GPIO 6
- EPD busy: GPIO 8
- EPD reset: GPIO 9
- EPD D/C: GPIO 10
- EPD CS: GPIO 11
- EPD clock: GPIO 12
- EPD MOSI: GPIO 13
- BOOT/Confirm: GPIO 0

Die V2-Zuordnung ist auf das vorhandene Board mit 8 MB Flash und 8 MB PSRAM ausgerichtet. V1 und V2 sind laut Hersteller nicht austauschbar.

## Abnahmekriterien

- Host-Build und 16 Tests erfolgreich.
- ESP-IDF-Projekt für `esp32s3` konfigurierbar.
- USB-Probe meldet Firmware 0.21.1, Revision 2 sowie DISPLAY und BUTTONS.
- E-Paper zeigt nach dem Flashen den BC2-Start-/Sperrbildschirm.
- BOOT-Taste erzeugt ein Bestätigen-Ereignis.
