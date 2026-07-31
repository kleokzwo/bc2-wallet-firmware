# Sprint 3 – Verbindlicher Entwicklungsplan

## Ausgangsstand

Arbeitsgrundlage ist ausschließlich `BC2 Cold Wallet v0.10.0`.

## Ziel

Ein konsistentes, wiederverwendbares Qt-Komponentensystem als Grundlage für alle Wallet-Seiten. Der Sprint verändert keine Wallet-, Netzwerk-, Kryptografie- oder Hardwarefunktion.

## Lieferumfang

- `Bc2Button` mit Primary-, Secondary-, Navigation- und Danger-Varianten
- `Bc2Card` als einheitlicher Inhaltscontainer
- `Bc2Header` für Seitentitel und Untertitel
- `Bc2Dialog` als einheitliche Dialogbasis
- `Bc2StatusBar` für neutrale, erfolgreiche, warnende und fehlerhafte Zustände
- `Bc2LoadingWidget` für Fortschritt und Busy-Zustände
- `Bc2QrWidget` als sichere Layout- und Payload-Basis
- zentrale Theme-Regeln für alle Komponenten
- Migration der bestehenden Oberfläche auf Button-, Card- und Header-Komponenten

## Abgrenzung

Die finale, scanbare QR-Erzeugung gehört zur vollständigen Receive-Seite in Sprint 4. Sprint 3 stellt dafür ausschließlich das Widget, die Größenregeln und die Payload-Schnittstelle bereit. Es wird bewusst kein nicht validierter QR-Encoder als Sicherheitsfunktion eingeführt.

## Definition of Done

- Komponenten besitzen jeweils eine klar erkennbare Aufgabe.
- Bestehende Hilfsfunktionen für Cards, Buttons und Seitenüberschriften sind entfernt.
- Bestehendes Verhalten bleibt unverändert.
- C17-Core und alle Tests bleiben grün.
- Qt-Build wird ausgeführt, sofern Qt 6.4 oder neuer verfügbar ist; fehlende Toolchain wird dokumentiert.
- README, Changelog, Testbericht und Sprint-Report sind aktualisiert.
- Vollständige v0.11.0-ZIP und SHA-256-Prüfsumme liegen vor.
