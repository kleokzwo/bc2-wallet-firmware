# Sprint 8 Report – Desktop Readiness

## Version

BC2 Cold Wallet v0.16.0

## Ergebnis

Die Desktop-App besitzt jetzt eine kleine zentrale `AppSettings`-Klasse. Sie speichert ausschließlich nicht vertrauliche Desktop-Einstellungen:

- Dark-/Light-Theme
- Fensterposition und Fenstergröße
- Electrum-Host
- Electrum-Port
- SSL-Schalter

Die UI enthält keine QSettings-Schlüssel und keine verteilte Persistenzlogik. Das hält MainWindow und Seiten einfach und bereitet die Desktop-App als verlässliche Referenz für den späteren Hardware-Wire-up vor.

## Nicht verändert

- Wallet-Core und Kryptografie
- Seed- und Schlüsselmodell
- PSBT-Sicherheitsregeln
- ESP32-GPIOs und E-Paper-Treiber
- Hardware-Signierung
- USB-Kommandoumfang

## Prüfung

Der C17-Core, die CLI und alle Host-Tests wurden aus einem sauberen Build-Verzeichnis gebaut. Der Qt-6-Build ist nur möglich, wenn Qt 6.4+ auf dem Build-System installiert ist; diese Umgebung enthält keine Qt-Entwicklungspakete.
