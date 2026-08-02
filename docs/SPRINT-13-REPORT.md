# Sprint-13-Abschlussbericht – v0.21.1

## Ergebnis

Der Hardware-Sicherheits-Gate aus v0.21.0 wurde für das bestätigte Waveshare ESP32-S3-ePaper-1.54 V2 aufgehoben.

## Implementiert

- eigenes, kleines ESP-IDF-E-Paper-Modul ohne Qt- oder Arduino-Abhängigkeit
- SPI- und GPIO-Anbindung für das 200x200-Panel
- Text-Framebuffer mit kompakter 5x7-Schrift
- Start-, Sperr- und Statusansichten auf dem physischen Display
- BOOT-Taste als vorläufige Bestätigen-Taste
- 35-ms-Entprellung
- Board-Revision V2 in der USB-Identität
- Fähigkeiten DISPLAY und BUTTONS
- Behebung der ESP-IDF-5.5-Compilerwarnung beim USB-Serial/JTAG-Treiber

## Sicherheitsgrenzen

Unverändert ausgeschlossen:

- Seed-Erzeugung für reale Nutzung
- Speicherung privater Schlüssel
- Signierung
- Transaktions-Broadcast
- Wi-Fi und Bluetooth
- microSD-Nutzung

Die PWR-Taste wird nicht als Wallet-Navigation verwendet, da sie Teil der Stromversorgungsschaltung ist. Für diesen Bring-up dient ausschließlich BOOT als Bestätigen-Taste.

## Prüfung

- C17/C++17 Host-Build erfolgreich
- 16/16 Host-Tests erfolgreich
- ESP-IDF-Hardware-Build muss auf dem bereits eingerichteten Kali-System des Testers ausgeführt werden
- physische Display- und Tastenprüfung erfolgt nach Flashen auf dem Zielgerät
