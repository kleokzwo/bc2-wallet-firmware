# Sprint-11-Abschlussbericht – v0.20.0

## Ergebnis
v0.20.0 ergänzt die erste vollständige Watch-only-Send-Vorbereitung. Synchronisierte UTXOs können automatisch ausgewählt, Gebühren und Wechselgeld berechnet und als unsigned PSBT exportiert werden.

## Implementiert
- neues C17-Modul `bc2_transaction`
- BC2-Adressprüfung und ScriptPubKey-Erzeugung
- deterministische Coin Selection
- Gebühren- und Dust-Logik
- PSBT-v0-Erzeugung mit witness_utxo
- neue Send-Maske im Qt-Simulator
- PSBT-Dateiexport und Transaktionsvorschau
- Test `test_transaction` mit End-to-End-Prüfung über den vorhandenen PSBT-Parser

## Bewusste Grenzen
- keine Signierung
- kein Broadcast
- keine manuelle UTXO-Auswahl; v0.20.0 verwendet eine einfache deterministische Auswahl
- keine externe Fee-Schätzung; der Nutzer setzt sat/vB selbst
- Hardware-Bestätigung bleibt vorbereitet, aber deaktiviert

## Qualität
KISS: lineare, nachvollziehbare Coin Selection ohne unnötige Optimierer.
SOLID: Planung/Serialisierung im Core; UI und Netzwerkdaten bleiben getrennt.
Clean Code: begrenzte Datentypen, klare Statuscodes und reproduzierbare Tests.
