# Sprint-16-Abschlussbericht – v0.21.4

## Ursache
Die vorherigen BC2-Hotfixes verwendeten nicht die Pinbelegung und Treibersequenz der funktionierenden offiziellen Waveshare-V2-Demo.

## Umsetzung
- Display-BSP auf die offizielle V2-Belegung umgestellt
- E-Paper-Stromversorgung über GPIO 6 aktiviert
- offizielle 159-Byte-Vollrefresh-LUT integriert
- offizielle Fenster-, Cursor-, Reset- und Vollrefresh-Sequenz portiert
- bestehende schlanke BC2-Textdarstellung beibehalten
- Firmware-/Desktop-Version auf 0.21.4 erhöht

## Prüfung
Die Host-Suite wird mit dem bestehenden Buildskript ausgeführt. Der physische Displaytest erfolgt auf dem Waveshare ESP32-S3-ePaper-1.54 V2 mit ESP-IDF 5.5.3.
