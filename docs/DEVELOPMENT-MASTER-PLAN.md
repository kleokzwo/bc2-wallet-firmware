# BC2 Cold Wallet – Development Master Plan

## Übergabeprotokoll v0.9.0 → v0.9.1

Dieses Dokument ist die verbindliche Entwicklungsrichtlinie für alle folgenden Versionen.

## 1. Projektstatus

Arbeitsgrundlage für Sprint 1 war `BC2 Cold Wallet v0.9.0`. Neue Arbeiten bauen immer auf der jeweils zuletzt abgeschlossenen vollständigen Version auf. Architektur und bestehende Funktionen werden nicht ohne dokumentierte Entscheidung ersetzt.

## 2. Projektvision

Ziel ist eine vollständig quelloffene BC2-Only-Hardware-Wallet mit eigenem C17-Wallet-Core, Qt-Desktop-Simulator, eigener Firmware sowie konsistenter UI und UX. Das Produkt ist kein Fork einer bestehenden Wallet.

## 3. Zielhardware

Verbindliche Referenzplattform:

```text
Waveshare ESP32-S3
1.54" E-Paper AIoT Development Board
```

Die Firmware wird ausschließlich mit ESP-IDF entwickelt. Arduino ist nicht Teil der Architektur.

## 4. Rollen

Der Product Owner entscheidet über Produktumfang, testet Lieferstände und gibt Feedback. ChatGPT übernimmt Architektur, Implementierung, Tests, Refactoring und technische Dokumentation.

## 5. Entwicklungsregeln

- KISS: immer die einfachste tragfähige Lösung wählen.
- Code wird für Menschen geschrieben und muss schnell verständlich sein.
- Eine Funktion erfüllt möglichst genau eine Aufgabe.
- Namen beschreiben ihren Zweck.
- Kommentare erklären Gründe und Randbedingungen, nicht offensichtliche Abläufe.
- Keine unbenannten Konstanten für fachliche Werte.
- Gemeinsame Logik existiert nur einmal.
- Jedes Modul besitzt eine klar erkennbare Verantwortung.
- Jeder Zwischenstand bleibt kompilierbar.
- Jeder Sprint endet mit Build, Tests und Dokumentation.

## 6. Architektur

```text
Desktop → Qt → HAL → Wallet Core → HAL → ESP32 → Hardware
```

Der C17-Wallet-Core bleibt plattformunabhängig und wird sowohl vom Simulator als auch von der Hardware verwendet.

## 7. Sprintplan

1. Repository bereinigen, ohne neue Features.
2. Modernes Qt-Grundgerüst, Navigation, responsive Layouts und Theme-System.
3. Wiederverwendbare UI-Komponenten.
4. Vollständige Wallet-Seiten.
5. Navigation, Animationen, Icons und Design-Polish.
6. Hardware-Vorbereitung und HAL-Abgleich.
7. Waveshare-Integration für Display, Buttons, Flash und USB.
8. Hardware-UI.
9. Wallet-Funktionen für Seed, PIN, Backup, Restore und Signing.
10. Pre-Release mit vollständigen Tests, Review und Optimierung.

## 8. Definition of Done

Ein Sprint ist nur abgeschlossen, wenn:

- das Projekt im verfügbaren Zielsystem kompiliert,
- alle automatisierten Tests grün sind,
- der Code einfacher und lesbarer wurde,
- unnötige Duplikate entfernt wurden,
- Dokumentation und Changelog aktualisiert sind,
- ein Sprint-Report und eine vollständige ZIP vorliegen.

Nicht verfügbare Toolchains müssen im Sprint-Report ausdrücklich genannt werden.

## 9. UI-Philosophie

Die Desktop-Wallet soll wie hochwertige Endnutzer-Software wirken: modern, minimalistisch, ruhig, klar, responsiv und konsistent. Light Mode und Dark Mode gehören zum Zielbild. Die Hardware übernimmt dieselbe visuelle und sprachliche Produktidentität, angepasst an das E-Paper-Display.

## 10. Arbeitsweise für neue Chats

1. Dieses Dokument und die zuletzt abgeschlossene ZIP lesen.
2. Den nächsten offenen Sprint bestimmen.
3. Ausschließlich an diesem Sprint arbeiten.
4. Änderungen nach den KISS-Regeln implementieren.
5. Build und Tests ausführen.
6. README, Changelog und Sprint-Report aktualisieren.
7. Eine vollständige neue Versions-ZIP erzeugen.
8. Erst anschließend den nächsten Sprint beginnen.
