# Sprint 9 CI Hotfix 2 – Verbindlicher Entwicklungsplan

## Ausgangslage

Die Desktop-Builds von v0.17.1 sind auf Linux, Windows und macOS fehlgeschlagen.
Der Windows-Log belegt eine falsche Annahme über den Chocolatey-Installationspfad von OpenSSL.

## Ziel

Die Desktop-CI muss plattformspezifische Abhängigkeiten ohne fest geratene Installationspfade auflösen und bei einem Fehler verwertbare Diagnoseprotokolle ausgeben.

## Verbindlicher Umfang

1. Windows verwendet OpenSSL über vcpkg und dessen CMake-Toolchain.
2. macOS exportiert den Homebrew-Prefix von OpenSSL ausdrücklich.
3. Linux installiert alle bekannten nativen Qt-/OpenSSL-Abhängigkeiten.
4. Jeder Job gibt bei Fehlern CMake- und Build-Logs aus.
5. Der C17-Core behält Warnungen als Fehler.
6. Das plattformabhängige Qt-Frontend verwendet weiterhin hohe Warnstufen, aber kein `-Werror` beziehungsweise `/WX`.
7. Qt-Deployment wird nur auf den offiziell unterstützten Paketplattformen Windows und macOS ausgeführt.

## Definition of Done

- Host-Core kompiliert.
- Alle Host-Tests bestehen.
- Workflow enthält keine festen OpenSSL-Installationspfade unter Windows.
- Alle drei Jobs besitzen eine eigene Fehlerdiagnose.
- Dokumentation und Changelog sind aktualisiert.
- Vollständige Repository-ZIP und SHA-256-Prüfsumme werden erzeugt.
