# Verbindlicher Entwicklungsplan – Sprint 16 / v0.21.4

## Ziel
Den nachweislich funktionierenden offiziellen Waveshare-V2-E-Paper-Unterbau in die BC2-Firmware integrieren.

## Verbindlicher Umfang
- offizielle V2-Pins verwenden: PWR 6, BUSY 8, RST 9, DC 10, CS 11, SCLK 12, MOSI 13
- offizielle Reset-, LUT-, Fenster-, Cursor- und Refresh-Sequenz übernehmen
- BC2-UI und Wallet-Core unverändert über die HAL anbinden
- USB, NVS, RNG und BOOT-Taste nicht verschlechtern
- keine Seed- oder Signierungsfunktionen ergänzen

## Abnahme
- Host-Tests vollständig erfolgreich
- ESP-IDF 5.5.3 Hardware-Build auf dem Zielsystem
- sichtbarer BC2-Sperrbildschirm nach dem Flashen
