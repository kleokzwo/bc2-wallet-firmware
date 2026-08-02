# Verbindlicher Entwicklungsplan – Sprint 15 / v0.21.4

## Ziel

Den physischen Vollrefresh des Waveshare ESP32-S3-ePaper-1.54 V2 zuverlässig auslösen und nur nach einer tatsächlich beobachteten BUSY-Phase als erfolgreich melden.

## Verbindlicher Umfang

1. BUSY-Signal gemäß V2-Controller als aktiv HIGH behandeln.
2. Nach `MASTER_ACTIVATION` zuerst auf BUSY=HIGH und danach auf BUSY=LOW warten.
3. Getrennte Zeitlimits für BUSY-Aktivierung und BUSY-Freigabe verwenden.
4. Vollrefresh-Steuerbyte `0xF7` entsprechend der Waveshare-V2-Sequenz verwenden.
5. Border-Waveform auf `0x05` setzen.
6. Aussagekräftige Diagnosewerte protokollieren.
7. `display=ready` erst nach erfolgreicher Initialisierung führen.
8. Seed, Private Keys und Signierung bleiben deaktiviert.

## Abnahmekriterien

- Hardware-Build mit ESP-IDF 5.5.3 erfolgreich.
- Das Log meldet eine echte BUSY-HIGH-Phase und anschließend BUSY-LOW.
- Ein Vollrefresh dauert plausibel länger als wenige Millisekunden.
- Der BC2-Sperrbildschirm wird physisch sichtbar.
- USB-Geräteprobe bleibt funktionsfähig.
