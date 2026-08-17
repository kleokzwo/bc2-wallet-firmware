# Sprint 5 Report – Navigation und Design Polish

## Version

BC2 Cold Wallet v0.13.0

## Ausgangsprüfung

Die vollständige v0.12.0 wurde vor Änderungen geprüft. Repository-Struktur, Core, Simulator, CMake, Tests, Masterplan und Sprint-4-Dokumentation waren vorhanden. Der nächste offene Punkt des verbindlichen Masterplans war Sprint 5.

## Umsetzung

- Aktive Seite wird in der Sidebar eindeutig hervorgehoben.
- Navigationspunkte besitzen einfache, lokale Textsymbole ohne externe Asset-Abhängigkeit.
- Die Sidebar wechselt unterhalb des definierten Breakpoints in einen kompakten Symbolmodus.
- Tooltips und Accessible Names erhalten im kompakten Modus die vollständige Bezeichnung.
- Ein eigener Scrollbereich hält alle Navigationsziele bei geringer Fensterhöhe erreichbar.
- Der PageRouter blendet neue Seiten mit einer 160-ms-Opacity-Transition ein.
- Fokus-, Hover-, Eingabe-, Scrollbar-, Tooltip- und aktive Navigationszustände wurden im Theme vereinheitlicht.
- Eine sichtbare Kennzeichnung weist auf die Entwicklungsumgebung hin.
- Versionsangaben wurden auf 0.13.0 aktualisiert.

## Architekturentscheidung

Die Übergangslogik liegt ausschließlich im PageRouter. Dadurch müssen einzelne Seiten keine Animation kennen und die Navigation bleibt zentral. Der aktive Zustand wird über die registrierte PageRouter-Seite auf die Navigationsbuttons übertragen. Es wurden keine direkten QStackedWidget-Indizes eingeführt.

## Sicherheitsentscheidung

Die Animation verändert nur die Deckkraft der bereits ausgewählten Seite. Sie verändert keine Daten, bestätigt keine Aktion und verzögert keine Geräteentscheidung. Externe Icon-Pakete wurden vermieden, damit keine zusätzliche Liefer- oder Lizenzabhängigkeit entsteht.

## Prüfungen

- CMake-Konfiguration Core: erfolgreich
- Wallet-Core Build: erfolgreich
- CLI Build: erfolgreich
- Warnungen als Fehler: erfolgreich
- CTest: 11/11 bestanden
- Qt-Konfigurationsversuch: nicht möglich, weil Qt 6.4+ in der Ausführungsumgebung nicht installiert ist
- Quellstruktur und CMake-Quellenliste: geprüft

## Nächster Sprint

Sprint 6: Hardware-Vorbereitung. HAL und gemeinsame Abläufe werden gegen die Desktop- und ESP32-Ziele geprüft und synchronisiert. Es werden noch keine unbestätigten Display-GPIOs aktiviert.
