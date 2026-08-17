# Verbindlicher Entwicklungsplan v0.7.0

## Ziel
PSBT-v0-Dateien deterministisch und begrenzt einlesen, alle Eingangs- und Ausgangsbeträge darstellen, Gebühren nur aus bekannten UTXOs berechnen und Wechselgeld ausschließlich gegen bekannte öffentliche Wallet-Scripts prüfen.

## Sicherheitsregeln
- Keine Signatur bei fehlenden UTXO-Beträgen.
- Keine Schätzung von Gebühren oder Wechselgeld.
- PSBT-, Map-, Input-, Output- und Scriptgrößen sind begrenzt.
- Aktuell unterstützt: PSBT v0 und native P2WPKH-Transaktionen mit `witness_utxo`.
- `non_witness_utxo`, Taproot, Multisig, PSBT v2 und Signieren folgen erst nach separater Implementierung und Tests.

## Lieferumfang v0.7.0
1. Vollständige unsigned-Transaction aus der globalen PSBT-Map.
2. Input-Prevouts, Sequenzen und `witness_utxo`-Beträge.
3. Outputs mit Betrag, Script und P2WPKH-Adresse.
4. Total Input, Total Output und exakte Gebühr.
5. Wechselgelderkennung über bekannte Watch-only-Scripts.
6. Qt-Transaktionsansicht mit klarer Blockierung bei unvollständigen Daten.
7. Automatisierte C-Tests für Beträge, Gebühr und Wechselgeld.
