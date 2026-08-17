# Sprint-12-Abschlussbericht – v0.21.0

## Ergebnis
Der erste reale Hardware-Sprint ist als sichere Bring-up-Version abgeschlossen. Die Firmware kann über USB Serial/JTAG identifiziert und geprüft werden. Ein fehlender microSD-Slot oder eine fehlende microSD-Karte blockiert diesen Stand nicht.

## Implementiert
- C17-USB-Stream-Decoder mit dauerhaftem Empfangspuffer
- Verarbeitung fragmentierter Frames über mehrere USB-Lesevorgänge
- Verarbeitung mehrerer direkt aufeinanderfolgender Frames
- Wiederherstellung nach Stördaten vor dem BC2-Magic-Header
- Gerätefähigkeit `GET_CAPABILITIES`
- Firmwareversion 0.21.0 in der Hardwareantwort
- Python-Probeprogramm `tools/bc2_device_probe.py`
- Build- und Flash-Skripte unter `scripts/`
- Fähigkeiten für USB, NVS und Hardware-RNG

## Bewusst noch nicht aktiviert
Das physische E-Paper und die Tasten bleiben im BSP sicher gesperrt. Im Repository ist die Board-Revision weiterhin `UNKNOWN`. Waveshare-Revisionen können unterschiedliche Beispielprogramme und Pinbelegungen verwenden. Die Aktivierung erfolgt erst nach Ablesen oder Fotografieren der konkreten Revision.

## Prüfungen
- CMake Release Host-Build: erfolgreich
- 16/16 Host-Tests: erfolgreich
- Python-Syntaxprüfung des Probeprogramms: erfolgreich
- ESP-IDF-Hardware-Build: nicht ausgeführt, weil ESP-IDF in der Arbeitsumgebung nicht installiert ist
- Qt-Desktop-Build: nicht ausgeführt, weil Qt 6 in der Arbeitsumgebung nicht installiert ist

## Nächster Hardware-Schritt
1. Board per USB-C anschließen.
2. ESP-IDF 5.5 oder neuer aktivieren.
3. `./scripts/build-hardware.sh` ausführen.
4. `./scripts/flash-hardware.sh` ausführen.
5. `python -m pip install pyserial` installieren.
6. Port mit `python tools/bc2_device_probe.py --list` ermitteln.
7. Probe mit `python tools/bc2_device_probe.py --port <PORT>` starten.
8. Board-Revision bestätigen; danach Display und Tasten gezielt aktivieren.
