# Repository-Prüfung v0.7.0

## Bestätigter Stand

- C17-Wallet-Core und Qt-6-Simulator bauen im GitHub Codespace.
- 9 von 9 Tests bestehen.
- Watch-only-Electrum, Gerätezustände und PSBT-v0-Prüfung sind integriert.
- Der Qt-MOC-Linkerfehler wurde durch Entfernen unnötiger `Q_OBJECT`-Makros behoben.

## Vor v0.8.0 fehlend

- gemeinsame Schnittstellen für Display, Tasten, Zeit, Zufall, Speicher und USB,
- simulatorseitiges Backend dieser Schnittstellen,
- feste E-Paper-Frame-Struktur,
- automatisierte HAL-Tests,
- dokumentierte Grenze zwischen Core und Zielhardware.

## Entscheidung

Die Architektur bleibt unverändert. v0.8.0 ergänzt die Hardware-Abstraktion unterhalb des bestehenden Wallet-Cores und ersetzt keine vorhandene Wallet-Funktion.
