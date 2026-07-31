# Qt Foundation v0.10.0

## Zweck

Dieses Dokument beschreibt das verbindliche Desktop-Grundgerüst ab Sprint 2.

## Verantwortlichkeiten

### MainWindow

- besitzt die Anwendungs-Shell
- verbindet bestehende Controller und Models
- koordiniert Navigation und responsive Darstellung
- enthält keine Theme-Definitionen

### PageRouter

- registriert Seiten mit sprechenden Seitentypen
- öffnet Seiten ohne Magic Numbers
- kapselt das `QStackedWidget`

### ThemeManager

- verwaltet das aktive Theme
- stellt Dark Mode und Light Mode bereit
- wendet das zentrale Qt-Stylesheet an

### DesignTokens

- enthält zentrale Größen, Abstände, Radien und Breakpoints
- verhindert verteilte Magic Numbers in der UI-Infrastruktur

## Erweiterungsregel

Neue Seiten werden künftig über `PageRouter::Page` benannt und registriert. Direkte Aufrufe von `QStackedWidget::setCurrentIndex()` sind im Anwendungscode nicht erlaubt.

Neue globale Farben und Abstände werden ausschließlich im Theme-System beziehungsweise in `DesignTokens` gepflegt.
