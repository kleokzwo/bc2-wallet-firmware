# Verbindlicher Entwicklungsplan – Sprint 11 / v0.20.0

## Ziel
Eine sichere Watch-only-Send-Funktion, die aus synchronisierten UTXOs einen prüfbaren unsigned PSBT-Entwurf erzeugt. Es findet keine Signierung und keine Übertragung ins Netzwerk statt.

## Lieferumfang
1. BC2-Mainnet-Adressprüfung für SegWit v0 und P2PKH.
2. Deterministische, einfache Coin Selection aus vorhandenen Watch-only-UTXOs.
3. Gebührenberechnung über eine vom Benutzer gesetzte sat/vB-Rate.
4. Dust-Behandlung und Wechselgeldentscheidung.
5. Erzeugung eines PSBT-v0-Entwurfs inklusive witness_utxo für jeden Eingang.
6. Desktop-Maske für Empfänger, Betrag und Gebührenrate.
7. Vorschau von Eingangszahl, Betrag, Gebühr, Wechselgeld und geschätzten vBytes.
8. Export als binäre `.psbt`-Datei.
9. Wiederverwendung des vorhandenen PSBT-Prüfdialogs.

## Sicherheitsgrenzen
- keine Seeds oder Private Keys im Desktop
- keine Signierung
- kein Broadcast
- keine Hardware-Implementierung vor physischer Verfügbarkeit
- Hardware muss später alle kritischen Werte unabhängig bestätigen

## Architektur
Der Transaktionsplaner liegt im C17-Core. Qt übernimmt ausschließlich Eingabe, Darstellung, Dateiauswahl und den Zugriff auf öffentliche Watch-only-UTXOs. Die API bleibt für ESP32 und andere Frontends nutzbar.
