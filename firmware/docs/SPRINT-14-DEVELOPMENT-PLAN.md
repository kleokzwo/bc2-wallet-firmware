# Verbindlicher Entwicklungsplan – Sprint 14 / v0.21.4

## Ziel

Das physische 200×200-E-Paper des Waveshare ESP32-S3-ePaper-1.54 V2 muss nach dem Flashen sichtbar und reproduzierbar den BC2-Gerätezustand anzeigen.

## Verbindlicher Umfang

1. Falsche v0.21.1-GPIO-Zuordnung vollständig entfernen.
2. Offizielle V2-Pinbelegung verwenden: BUSY 21, RST 11, D/C 13, CS 12, SCLK 10, SDI 8.
3. Keine erfundene Display-Power-Leitung schalten; das Panel ist auf V2 mit EPD3V3 versorgt.
4. V2-kompatible Reset-, RAM-Fenster-, Cursor- und Vollrefresh-Sequenz verwenden.
5. Framebuffer in einer zusammenhängenden SPI-Übertragung senden.
6. `display=ready` erst nach erfolgreicher Controller-Initialisierung melden.
7. Beginn und Abschluss jedes Vollrefreshs protokollieren.
8. USB, NVS, RNG, Wallet-Core und Sicherheitsgrenzen unverändert erhalten.

## Abnahmekriterien

- ESP-IDF 5.5.3 kompiliert ohne neue Warnungen.
- Startlog meldet die sechs tatsächlichen E-Paper-GPIOs.
- Log meldet `Starting full refresh` und danach `Full refresh completed`.
- Das Display flackert beim Vollrefresh und zeigt den BC2-Sperrbildschirm.
- Geräteprobe meldet Firmware 0.21.4, Revision 2 sowie DISPLAY und BUTTONS.
- Keine Seeds, privaten Schlüssel oder Signaturfunktionen werden aktiviert.
