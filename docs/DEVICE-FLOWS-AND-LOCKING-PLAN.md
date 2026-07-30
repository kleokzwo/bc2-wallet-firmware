# Verbindlicher Entwicklungsplan – v0.6.0 Geräteabläufe und Sperrlogik

## Ziel

v0.6.0 stellt einen einzigen, wiederverwendbaren Gerätezustand für Simulator und spätere Firmware bereit. Sicherheitsentscheidungen dürfen nicht ausschließlich in der Qt-Oberfläche liegen.

## Verbindliche Zustände

1. Boot
2. Setup erforderlich
3. Gesperrt
4. Entsperrung läuft
5. Zeitgesteuerte Sperrphase nach zu vielen Fehlversuchen
6. Dashboard
7. Empfangsadresse prüfen
8. Transaktion prüfen
9. Einstellungen prüfen
10. Fehlerzustand

## Verbindliche Regeln

- Nach dem Boot ist ein initialisiertes Gerät immer gesperrt.
- Fünf aufeinanderfolgende Fehlversuche starten eine zeitgesteuerte Sperre.
- Die erste Sperre dauert 30 Sekunden; weitere Sperrzyklen wachsen exponentiell bis maximal 24 Stunden.
- Ein erfolgreicher Entsperrvorgang setzt Fehlversuche und Sperrstufe zurück.
- Eine entsperrte Sitzung wird nach fünf Minuten ohne Aktivität automatisch gesperrt.
- Empfangsadresse, Transaktion und Einstellungen besitzen jeweils definierte Bestätigungs- und Abbruchpfade.
- Ein globales Sperrereignis hat in allen entsperrten Prüfzuständen Vorrang.
- Ein schwerer Fehler führt in einen separaten Fehlerzustand; eine Wiederherstellung endet stets gesperrt.
- Die Zustandsmaschine prüft keine PIN. Sie erhält nur das Ergebnis einer späteren sicheren PIN-Komponente.

## Lieferung v0.6.0

- Erweiterte C17-Zustandsmaschine
- deterministische Zeitübergabe in Millisekunden
- Session-Timeout
- exponentielle Sperrzeiten
- explizite Sicherheitsaktionen
- Qt-Controller als einzige Brücke zwischen UI und C17-Zustandsmaschine
- Sperrbildschirm im Simulator
- ausschließlich dokumentierter Simulator-Test-PIN `2468`
- automatisierte Core-Tests für alle zentralen Abläufe

## Noch nicht produktionsreif

- Der Test-PIN ist keine sichere PIN-Speicherung oder PIN-Verifikation.
- Es gibt noch keinen Secure Element, keinen Anti-Hammering-Zähler in nichtflüchtigem Speicher und keinen manipulationssicheren Monotonic Counter.
- Ein Neustart des Simulators setzt Sperrzeiten zurück. Auf echter Hardware müssen Sperrzustand und Fehlversuche manipulationssicher persistiert werden.
- Die Transaktionsbestätigung löst weiterhin keine Signatur aus.

## Nächste verbindliche Stufe

v0.7.0 erweitert den PSBT-Parser um vollständige Transaktionsdetails, Beträge, Gebühren, Wechselgeldprüfung und eine mehrseitige Geräteanzeige. Signieren bleibt deaktiviert, bis diese Prüfungen vollständig getestet sind.
