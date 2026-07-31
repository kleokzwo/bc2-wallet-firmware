# Sprint 5 – Verbindlicher Entwicklungsplan

## Zielversion

BC2 Cold Wallet v0.13.0

## Sprint-Ziel

Die Desktop-Wallet erhält eine konsistente, hochwertige Navigation und ruhige visuelle Übergänge. Dieser Sprint verändert ausschließlich UI, UX und Präsentation des Qt-Simulators. Wallet-Core, Kryptografie, Netzwerk, HAL, Gerätezustand und Sicherheitsgrenzen bleiben unverändert.

## Verbindlicher Umfang

1. Aktiven Navigationspunkt eindeutig markieren.
2. Navigation mit einfachen, verständlichen Symbolen ergänzen.
3. Sidebar bei schmaleren Fenstern in einen kompakten Modus schalten.
4. Navigation bei kleinen Fensterhöhen scrollbar und vollständig erreichbar halten.
5. Seitenwechsel mit einer kurzen, nicht ablenkenden Einblendung versehen.
6. Fokus-, Hover-, Eingabe- und Scrollbar-Zustände im Theme vereinheitlichen.
7. Entwicklungsstatus sichtbar, aber zurückhaltend kennzeichnen.
8. Version, README, Changelog, Tests und Sprint-Bericht aktualisieren.

## Ausdrücklich ausgeschlossen

- keine neuen Wallet-Funktionen
- keine Seed-Verarbeitung
- keine Signierung
- keine QR-Encoder-Implementierung
- keine ESP32- oder USB-Änderungen
- keine Animation sicherheitskritischer Bestätigungsdaten
- keine externen Icon- oder Theme-Abhängigkeiten

## Definition of Done

- Navigation ist vollständig über den PageRouter gesteuert.
- Aktive Seite ist visuell erkennbar.
- Kompakter Modus bleibt per Tooltip und Accessible Name verständlich.
- Alle Navigationsziele bleiben bei geringer Fensterhöhe erreichbar.
- Übergänge blockieren keine Bedienung und dauern höchstens 160 ms.
- C17-Core und CLI kompilieren mit Warnungen als Fehler.
- Alle vorhandenen Tests bestehen.
- Qt-Konfigurationsstatus ist dokumentiert.
- README, Changelog, Testnachweis und Sprint-Report sind aktuell.
- Eine vollständige, saubere ZIP wird erzeugt.
