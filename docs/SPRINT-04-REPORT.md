# Sprint 4 Report – Vollständige Wallet-Seiten

## Version

BC2 Cold Wallet v0.12.0

## Ergebnis

Die fehlenden Desktop-Seiten wurden als getrennte Qt-Klassen ergänzt und über den bestehenden PageRouter eingebunden. Die bestehende Wallet-, Kryptografie-, Netzwerk-, Geräte- und HAL-Logik wurde nicht verändert.

## Neue Seiten

- Transaction: Einstieg in die vorhandene PSBT-Prüfung
- History: leere Watch-only-Verlaufstabelle mit sicherem Ausgangszustand
- Settings: nicht sicherheitskritische Desktop-Einstellungen
- About: Architektur, Version und Sicherheitsmodell
- Recovery: ausschließlich gerätegeführter Platzhalter ohne Seed-Eingabe
- Backup: Sicherheitsregeln für spätere Hardwareabläufe
- Error: neutraler Fehlerzustand ohne vertrauliche Details
- Factory Reset: deaktivierter Gefahrenbereich ohne Löschfunktion

## Architektur

Die Seiten liegen in `simulator/src/pages/walletpages.*`. Sie verwenden Bc2Header, Bc2Card, Bc2Button und Bc2StatusBar. Direkte QStackedWidget-Indizes wurden nicht eingeführt.

## Sicherheitsentscheidungen

Recovery-Wörter dürfen nicht am Desktop eingegeben werden. Factory Reset bleibt deaktiviert, bis Geräte-PIN und physische Bestätigung auf der ESP32-S3-Hardware implementiert sind. Die bestehende PSBT-Prüfung signiert weiterhin nicht.

## Prüfungen

- Host-Core Build: erfolgreich
- CLI Build: erfolgreich
- Compiler-Warnungen als Fehler: erfolgreich
- CTest: 11/11 bestanden
- Qt-Konfiguration: in der Ausführungsumgebung nicht möglich, da Qt 6.4+ nicht installiert ist

## Nächster Sprint

Sprint 5: Navigation, Animationen, Transitions, Icons und Design Polish. Es werden keine Wallet-Core- oder Hardwarefunktionen vorgezogen.
