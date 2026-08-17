# Sprint 19 – v0.24.0 Zwei-Tasten-PIN

## Ziel

Die echte Waveshare-Hardware erhält eine bedienbare PIN-Oberfläche. PWR navigiert vorwärts, BOOT rückwärts und beide Tasten gemeinsam bestätigen die markierte Taste.

## Umsetzung

- `bc2_pin_entry`: kleine, plattformunabhängige Zustandslogik für vier PIN-Ziffern
- `[<]`: entfernt ausschließlich die zuletzt eingegebene Ziffer
- `bc2_navigation`: erkennt Zwei-Tasten-Chords und unterdrückt Folgeereignisse beim Loslassen
- Hardwareeingabe: PWR/BAT_KEY auf GPIO 18 und BOOT auf GPIO 0, jeweils aktiv LOW und getrennt entprellt
- Hardwareanzeige: 3×3-Ziffernfeld plus `[<]` und `0`; Auswahl durch einen kräftigeren Rahmen
- PIN-Puffer wird nach Erfolg oder Fehler sicher überschrieben

## Sicherheitsgrenze

Der fest eingebaute Wert `2468` ist nur ein Entwicklungstest. v0.24.0 speichert noch keine produktive PIN und enthält noch keine Root-PIN. Keine echten Wallet-Daten verwenden.

## Folgesprint

1. Geräte-PIN sicher einrichten und gesalzen/verlangsamt prüfen.
2. Transaktionsfreigabe an Geräte-PIN plus physische Bestätigung binden.
3. Empfangsadressfreigabe an Geräte-PIN plus physische Bestätigung binden.
4. Wallet-Neuanlage ausschließlich über eine getrennte Root-PIN erlauben.

## Hardware-Abnahmetest

1. Beide Tasten auf dem Sperrbildschirm drücken: PIN-Ansicht erscheint.
2. Obere Taste: Auswahl bewegt sich nach rechts.
3. Untere Taste: Auswahl bewegt sich nach links.
4. Beide Tasten: markierte Ziffer wird übernommen, ohne zusätzlichen Sprung.
5. `[<]`: die letzte PIN-Stelle verschwindet.
6. `2468`: Dashboard erscheint.
7. Falsche PIN: Gerät kehrt in den Sperrzustand zurück.
