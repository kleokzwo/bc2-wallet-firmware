# BC2 Cold Wallet v0.29.7 – Teststatus

## Hotfix-Prüfung

- Geräte-Service unter C17 mit `-Wall -Wextra -Wpedantic -Werror` bestanden.
- Navigationstest unter denselben strikten Warnungen bestanden.
- Terminaler Transaktionsstatus bleibt bei wiederholter Abfrage erhalten.
- Waveshare-I/O-Vertrag geprüft: USB-RX ist nichtblockierend und physische
  Eingabe wird vor USB verarbeitet.
- Vollständiger Qt- und ESP-IDF-Build ist auf dem Zielsystem auszuführen.

Stand: 2026-08-03

## Automatisch geprüft

- `test_device_service` und `test_device_flow` mit C17 und
  `-Wall -Wextra -Wpedantic -Werror` erfolgreich.
- Ergebniszustände `pending`, `approved`, `rejected` und `none` werden korrekt übertragen.
- Eine physische Ablehnung ist über die einzelne BOOT-Taste erreichbar.
- Aktive Prüfung bleibt auch nach Entnahme aus der Warteschlange gesperrt.
- Abgeschlossenes, nicht abgeholtes Ergebnis blockiert keine neue Prüfung.
- Die mitgelieferte Test-PSBT erzeugt mit der zentralen Mainnet-Konfiguration
  eine `bc1…`-Adresse, die die Hardwarevalidierung akzeptiert.
- Der PSBT-/Hardwarevalidierungs-Regressions­test besteht mit C17 und
  `-Wall -Wextra -Wpedantic -Werror`.
- Die Desktop-Abfrage enthält kein festes Bedienzeitlimit mehr: `PENDING`
  führt ausschließlich zu einer weiteren Statusabfrage.
- Versionswerte sind auf 0.29.7 vereinheitlicht.
- Kurzzeitig ausbleibende USB-Antworten während eines E-Paper-Refreshs werden erneut abgefragt.
- Offizielle Display-Komponenten bleiben unverändert; nur der BC2-BSP-Loop und
  seine USB-Fristen wurden korrigiert.

## Noch auf Zielsystem abzunehmen

- vollständiger Host-Build und 21/21 CTest-Tests
- Qt-6-Simulator mit SerialPort
- ESP-IDF-5.5.3-Hardware-Build
- USB -> PIN -> Transaktionsanzeige -> Bestätigung und Desktop-Erfolg
- BOOT-Abbruch -> eindeutige Ablehnung im Desktop
- Lesbarkeit aller Werte und Ghosting auf dem echten E-Paper

Keine echten Seeds oder Guthaben verwenden. Signierung und Broadcast bleiben
deaktiviert. v0.28.0 wurde zuvor auf echter Hardware bis zur
Transaktionsanzeige erfolgreich bestätigt.
