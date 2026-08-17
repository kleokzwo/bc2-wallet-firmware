# Sprint 1 Report – Repository Cleanup

## Version

`v0.9.1`

## Ziel

Das Repository ohne neue Produktfunktionen verständlicher und wartbarer machen. Schwerpunkt war die größte Quelldatei des Qt-Simulators.

## Durchgeführte Änderungen

- `simulator/src/mainwindow.cpp` von 551 auf 341 Zeilen reduziert.
- Aufbau der Shell und aller Seiten nach `simulator/src/mainwindow_pages.cpp` ausgelagert.
- UI-Aufbau und Laufzeitverhalten der Hauptklasse klar getrennt.
- Kompakte Einzeilen-Navigationsmethoden in lesbare Kontrollflüsse umgeschrieben.
- Simulator-Versionsanzeige von 0.8.0 auf 0.9.1 korrigiert.
- CMake-Projektversion auf 0.9.1 aktualisiert.
- Verbindlichen Development Master Plan ergänzt.
- README, Changelog und Testergebnis aktualisiert.

## Funktionsumfang

Keine neuen Features. Das Verhalten der bestehenden Wallet-, Netzwerk-, Watch-only-, Sperr- und PSBT-Funktionen wurde nicht erweitert.

## Build und Tests

- Wallet-Core und CLI: erfolgreich gebaut.
- Automatisierte Host-Tests: 11/11 bestanden.
- Qt-Simulator: in dieser Laufzeit nicht gebaut, weil Qt 6 nicht installiert ist und der Paketserver nicht erreichbar war.
- ESP-IDF-Hardwareprojekt: nicht gebaut, weil die ESP-IDF-Toolchain in dieser Laufzeit nicht installiert ist.

## Definition of Done

- Core kompiliert: erfüllt.
- Tests grün: erfüllt, 11/11.
- Code lesbarer: erfüllt.
- Große Datei verkleinert: erfüllt.
- Keine neuen Features: erfüllt.
- Dokumentation aktualisiert: erfüllt.
- Vollständige ZIP: erstellt.

Der Qt- und Hardware-Build bleibt eine dokumentierte Umgebungsgrenze und muss in einer passenden Toolchain erneut ausgeführt werden.
