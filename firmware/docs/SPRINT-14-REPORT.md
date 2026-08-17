# Sprint-14-Abschlussbericht – v0.21.4

## Ausgangslage

v0.21.1 startete auf der realen ESP32-S3-PICO-1-N8R8-Hardware und meldete das Display fälschlich als bereit. Das E-Paper blieb jedoch unverändert.

## Gefundene Ursache

Der Treiber verwendete eine falsche GPIO-Reihenfolge (GPIO 8–13) und schaltete zusätzlich GPIO 6 als vermeintliche Display-Stromversorgung. Laut offiziellem Waveshare-V2-Schaltplan sind die Signale anders verdrahtet: BUSY=21, RST=11, D/C=13, CS=12, SCLK=10 und SDI=8. Das Panel wird direkt über EPD3V3 versorgt.

## Umsetzung

- korrekte V2-Pinbelegung
- entfernte falsche Power-GPIO-Steuerung
- 200-ms-Hardware-Reset
- V2-kompatibles RAM-Fenster von Y=199 nach Y=0
- Cursorstart bei X=0/Y=199
- Initial-Update mit 0xB1
- Vollrefresh mit 0xC7
- manueller CS-Ablauf und zusammenhängende 5000-Byte-Framebufferübertragung
- eindeutige Refresh- und Pin-Diagnoselogs
- Versionsanhebung auf 0.21.4

## Prüfung in dieser Lieferumgebung

Die plattformunabhängigen Host-Tests werden ausgeführt. Ein physischer ESP-IDF-Flash kann nur auf dem angeschlossenen Board des Product Owners abschließend verifiziert werden.

## Sicherheit

Keine Seed-Erzeugung für reale Nutzung, keine Private-Key-Speicherung, keine Signierung, kein Broadcast, kein Wi-Fi und kein Bluetooth wurden aktiviert.
