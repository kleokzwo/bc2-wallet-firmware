# Sprint 9 – Desktop Build & Release

## Ziel

Die Desktop-App reproduzierbar für Linux, Windows und macOS bauen, testen und paketieren, ohne Hardware-Wire-up oder neue Wallet-Funktionen.

## Verbindlicher Umfang

- einheitliche Release-Paketierung über CMake/CPack
- Qt-Laufzeitabhängigkeiten beim Installieren bereitstellen
- lokale Linux- und Windows-Build-Skripte
- native CI-Builds für Linux, Windows und macOS
- vollständige Build-Dokumentation
- bestehende Core-Tests unverändert grün

## Nicht Bestandteil

- GPIO- oder E-Paper-Wire-up
- Seed- oder PIN-Funktionen
- Signierung
- Code-Signing und Notarisierung
- Installer mit Administratorrechten

## Definition of Done

- Core und Tests bauen lokal
- Desktop-Ziel ist paketierbar konfiguriert
- alle drei Betriebssysteme besitzen einen nativen CI-Pfad
- Windows-Paket ist portabel geplant
- Dokumentation erklärt die macOS-Grenze transparent
- README und Changelog sind aktualisiert
- vollständige ZIP und SHA-256 werden erzeugt
