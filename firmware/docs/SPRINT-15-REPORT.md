# Sprint-15-Abschlussbericht – v0.21.4

## Ausgangslage

v0.21.2 übertrug den Framebuffer und protokollierte einen Vollrefresh bereits nach 10–20 ms. Auf dem E-Paper blieb jedoch das vorherige Bild stehen. Das zeigte, dass LOW fälschlich sofort als abgeschlossener Refresh bewertet wurde.

## Umsetzung

- BUSY aktiv HIGH umgesetzt.
- Zweiphasige Refresh-Prüfung: BUSY muss zuerst HIGH werden und danach LOW.
- Aktivierungs-Timeout 1 Sekunde, Freigabe-Timeout 15 Sekunden.
- Vollrefresh-Steuerbyte von `0xC7` auf `0xF7` geändert.
- Border-Waveform auf `0x05` geändert.
- Diagnoseausgaben für Startpegel, Aktivierungszeit und Freigabedauer ergänzt.
- Version auf 0.21.4 erhöht.

## Sicherheitsstatus

Keine Seeds, keine Private Keys, keine Signierung und kein Broadcast. Wi-Fi und BLE bleiben deaktiviert.

## Hardware-Abnahme

Die finale Sichtprüfung muss auf dem physischen Waveshare-V2-Board erfolgen. Ein erfolgreicher Ablauf enthält mindestens:

```text
BUSY before activation: 0
BUSY entered active HIGH state after ... ms
BUSY released after ... ms
Full refresh completed
```
