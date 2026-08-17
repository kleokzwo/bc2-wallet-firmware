# Sichere Test-PSBT für v0.29.7

Wähle im Simulator die Datei `bc2-safe-test-transaction.psbt` aus.

Erwartete Anzeige:

- Eingänge: 1
- Ausgänge: 1
- Gesamteingang: 0,00100000 BC2
- Empfängerbetrag: 0,00099000 BC2
- Wechselgeld: 0,00000000 BC2
- Gebühr: 0,00001000 BC2

Die Datei enthält ausschließlich erfundene Transaktionsdaten. Sie ist nicht
signiert, besitzt keinen privaten Schlüssel und kann nicht an das Netzwerk
gesendet werden. Sie dient nur dazu, den v0.29.7-Ablauf auf Simulator und
Hardware zu prüfen.

Die Datei kann reproduzierbar neu erzeugt werden mit:

```bash
python3 tools/generate_test_psbt.py
```
