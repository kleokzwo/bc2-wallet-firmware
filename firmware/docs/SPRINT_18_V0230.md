# Sprint 18 – v0.23.0 Hardware UI

## Ergebnis

Die funktionierende Waveshare-Displaybasis aus v0.22.0 bleibt unverändert. v0.23.0 ergänzt ausschließlich BC2-Schichten oberhalb des BSP.

## Implementiert

- Sperrbildschirm für 200×200 Pixel mit BC2-Markierung, Statusleiste, Schloss, USB- und Batterieplatzhalter
- zentrale Zeichenfunktionen im BC2 Display Adapter
- Navigation Controller mit 35-ms-Hardwareentprellung aus dem BSP und semantischer Kurz-/Langdruck-Auswertung oberhalb des BSP
- Kurz drücken: bestätigen
- Lang drücken ab 800 ms: in entsperrten Zuständen sperren
- bestehende Zustandsmaschine und Bildschirmzuordnung weiterverwendet
- Full-/Partial-Refresh-Auswahl über `require_full_refresh`

## Bewusst unverändert

- `epaper_driver_bsp`
- `board_power_bsp`
- `waveshare_bsp.c`
- GPIO-Mapping
- BUSY-Handling
- Refresh-Sequenzen des Originaltreibers

## Sicherheitsstatus

Die UI simuliert den Einstieg in den Entsperrablauf. Eine produktive PIN-Eingabe, Schlüsselverwaltung und Signierung sind nicht Bestandteil dieses Sprints.

## Verifikation

- Host-Build und Host-Tests: siehe `TEST-RESULTS.txt`
- ESP-IDF-Kompilierung: siehe `TEST-RESULTS.txt`
- Physischer Displaytest: nach Flashen auf der Zielhardware erforderlich
