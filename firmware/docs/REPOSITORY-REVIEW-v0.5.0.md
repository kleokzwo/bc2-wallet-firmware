# Vollständige Prüfung des Ausgangsstands v0.5.0

## Vorhanden und weiterverwendet

- C17 Wallet-Core mit BIP39, BIP32, Adressen, Signaturgrundlagen und PSBT-Strukturprüfung
- Qt-6-Widgets-Simulator
- Electrum-SSL-Client und echte Watch-only-Abfragen
- Watch-only-Modell für Empfangs- und Wechseladressen
- E-Paper-Referenzansicht
- neun automatisierte Core-Tests

## Festgestellte Lücken bei Geräteabläufen

Die vorhandene Zustandsmaschine war eine erste Grundlage, aber noch nicht vollständig:

- nur einfacher Übergang nach fünf Fehlversuchen in einen allgemeinen Fehlerzustand
- keine zeitgesteuerte Sperrphase
- keine steigenden Sperrzeiten
- kein Session-Timeout
- keine Aktivitätsbehandlung
- keine expliziten Ergebnisse sicherheitskritischer Bestätigungen
- keine Wiederherstellung aus einem Fehlerzustand
- keine Integration der Zustandsmaschine in den Qt-Simulator
- Simulatorseiten waren trotz fehlender Geräteentsperrung direkt erreichbar

## Architekturentscheidung

Die bestehende Architektur wird nicht geändert. Die Zustandsmaschine bleibt in C17 und wird sowohl vom Qt-Simulator als auch später von der Hardware-Firmware verwendet. Qt enthält nur einen Controller und die Darstellung.
