# Sprint 17 – v0.21.7 Waveshare base-image integration

## Verbindliches Ziel

Den nachweislich funktionierenden Bildschreibpfad der offiziellen Waveshare-V2-Referenz übernehmen und jedes alte Demo-Bild zuverlässig ersetzen.

## Umfang

- Vor jedem Bildtransfer RAM-Fenster und Cursor explizit neu setzen.
- Aktuellen Framebuffer über Befehl `0x24` übertragen.
- Identischen Framebuffer über Befehl `0x26` als vorheriges/Basisbild übertragen.
- Erst danach den Vollrefresh über `0x22 / 0xC7 / 0x20` auslösen.
- Prüfsumme und Anzahl schwarzer Pixel protokollieren.
- Keine Änderungen an Seed-, Schlüssel- oder Signierungslogik.

## Abnahmekriterien

- ESP-IDF-Build für ESP32-S3 ist erfolgreich.
- Im Log erscheinen unterschiedliche, nichtleere BC2-Framebufferwerte.
- Beide RAM-Ebenen werden erfolgreich geschrieben.
- Der Refresh benötigt eine reale BUSY-Zeit.
- Das vorherige Waveshare-Demobild wird durch den BC2-Sperrbildschirm ersetzt.
