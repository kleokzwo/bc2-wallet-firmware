# Sprint 4 – Verbindlicher Entwicklungsplan

## Ausgangsbasis

BC2 Cold Wallet v0.11.0 ist die einzige Arbeitsgrundlage. Wallet-Core, Netzwerk-, Kryptografie-, Geräte- und HAL-Logik werden in diesem Sprint nicht verändert.

## Ziel

Alle im Development Master Plan geforderten Desktop-Wallet-Seiten werden als eigenständige, routbare Qt-Seiten bereitgestellt und verwenden ausschließlich das gemeinsame Design- und Komponentensystem aus Sprint 3.

## Verbindlicher Umfang

- Transaction
- History
- Settings
- About
- Recovery
- Backup
- Error
- Factory Reset
- Einbindung in PageRouter und Sidebar
- bestehende PSBT-Prüfung über die Transaction-Seite erreichbar
- klare Sicherheitsgrenzen für Recovery, Backup und Factory Reset

## Nicht Bestandteil

- keine Seed-Eingabe am Desktop
- keine produktive Signierung
- kein echter Factory Reset
- keine neue Wallet-Core-Funktion
- keine Hardwareansteuerung
- keine unvalidierte QR-Erzeugung

## Definition of Done

- alle Seiten besitzen eine eigenständige Klasse
- alle Seiten sind im PageRouter registriert
- Navigation ist ohne direkte Stack-Indizes möglich
- vorhandene PSBT-Prüfung bleibt erreichbar
- sicherheitskritische Aktionen bleiben deaktiviert oder gerätegebunden
- C17-Core und CLI kompilieren mit Warnungen als Fehler
- alle bestehenden Tests bestehen
- README, CHANGELOG, Sprint-Report und Testnachweis sind aktualisiert
- vollständige v0.12.0-ZIP und SHA-256-Prüfsumme liegen vor
