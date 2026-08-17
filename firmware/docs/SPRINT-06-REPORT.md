# Sprint 6 – Abschlussbericht

## Version

BC2 Cold Wallet v0.14.0

## Ergebnis

Sprint 6 ist abgeschlossen. Die zuvor getrennte Interpretation von Gerätezuständen, Tasten und Hardware-Bildschirmen wurde in die neue C17-Schicht `bc2_device_flow` verschoben.

## Implementierte Änderungen

- `bc2_device_flow_screen_for_state()` ordnet jeden Gerätezustand einem E-Paper-Bildschirm zu.
- `bc2_device_flow_event_from_button()` übersetzt Hardware-Tasten zentral in Zustandsmaschinen-Ereignisse.
- `bc2_device_flow_render()` rendert den aktuellen Zustand ausschließlich über die bestehende HAL.
- Die ESP32-App startet nun die gemeinsame Zustandsmaschine, führt einen RNG-Selbsttest aus, rendert Zustandsänderungen und verarbeitet den gemeinsamen Button-Flow.
- Der Qt-`DeviceController` besitzt einen Hardware-Button-Einstiegspunkt mit demselben C17-Mapping.
- `test_device_flow` prüft Boot, Entsperrstart, Dashboard-Navigation, Receive-Review, Rendering und Abbruch.

## Sicherheitsgrenzen

Die Boardrevision und damit die realen Display-/Button-GPIOs sind weiterhin nicht bestätigt. Die BSP-Treiber bleiben deshalb absichtlich deaktiviert. Wi-Fi, Bluetooth, produktive PIN-Prüfung, Seeds, private Schlüssel und Signierung wurden nicht aktiviert.

## Build und Tests

- Host-CMake-Konfiguration: erfolgreich
- C17-Wallet-Core: erfolgreich
- CLI: erfolgreich
- Warnungen als Fehler: erfolgreich
- Tests: 12/12 bestanden
- Qt-Konfiguration: nicht möglich, da Qt 6.4+ in der Arbeitsumgebung fehlt
- ESP-IDF-Build: nicht möglich, da `idf.py` in der Arbeitsumgebung fehlt

## Definition of Done

Alle innerhalb der verfügbaren Toolchains ausführbaren Prüfungen sind erfolgreich. Nicht verfügbare Qt- und ESP-IDF-Toolchains sind transparent dokumentiert.
