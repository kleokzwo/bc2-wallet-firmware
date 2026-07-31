# Sprint 2 Report — Modernes Qt-Grundgerüst

## Version

BC2 Cold Wallet v0.10.0

## Sprintziel

Ein modernes, erweiterbares Qt-6-Grundgerüst schaffen, ohne neue Wallet-Funktionen zu implementieren.

## Umgesetzt

- Zentrale `DesignTokens` für Fenstergrößen, Breakpoints, Abstände und Radien.
- Zentraler `ThemeManager` mit Dark Mode und Light Mode.
- Einheitliches Stylesheet wird nicht mehr in `MainWindow` verwaltet.
- Benannter `PageRouter` ersetzt direkte, fehleranfällige Seitenindizes.
- Responsive Sidebar mit kompakter Breite für kleinere Desktop-Fenster.
- Simulator- und Projektversion auf 0.10.0 aktualisiert.
- Bestehende Wallet-, Electrum-, PSBT- und Gerätezustandslogik unverändert beibehalten.

## Architektur

```text
MainWindow
├── ThemeManager
├── PageRouter
├── DesignTokens
├── bestehende Qt-Seiten
└── bestehende Controller und Models
```

Der `MainWindow` bleibt der Anwendungscontainer. Styling, Navigation und Designwerte besitzen jetzt jeweils eine klar abgegrenzte Verantwortung.

## Build und Tests

- C17-Wallet-Core: erfolgreich gebaut.
- CLI: erfolgreich gebaut.
- Host-Tests: 11/11 bestanden.
- Qt-Konfiguration: in der Ausführungsumgebung nicht möglich, da Qt 6.4+ nicht installiert ist.
- ESP-IDF-Build: nicht Teil dieses Desktop-Sprints.

## Definition of Done

- [x] Sprintziel umgesetzt
- [x] Keine neue Wallet-Funktion eingeführt
- [x] Code klarer strukturiert
- [x] Doppelte zentrale Stylinglogik vermieden
- [x] Core kompiliert
- [x] Alle verfügbaren Tests grün
- [x] README aktualisiert
- [x] Changelog aktualisiert
- [x] Sprint-Report erstellt
- [x] Vollständige Versions-ZIP erstellt

## Offener Prüfpunkt

Der Qt-Simulator muss in einer Umgebung mit Qt 6.4 oder neuer vollständig kompiliert und visuell geprüft werden. Die fehlende lokale Qt-Installation ist kein bestandener Qt-Build und wird deshalb ausdrücklich nicht als solcher dokumentiert.
