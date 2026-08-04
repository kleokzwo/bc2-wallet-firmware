# Sprint 20 – v0.25.0 PIN-Sicherheit

## Ergebnis

v0.25.0 entfernt den fest eingebauten Testwert und fuehrt eine persistente
sechsstellige Geräte-PIN ein. Beim ersten Entsperren wird der PIN zweimal eingegeben.
Nur Salt und PBKDF2-Pruefschluessel werden in NVS gespeichert.

## Fehlversuchsschutz

Die ersten zwei Fehler haben keine kuenstliche Pause. Danach gelten 5 s, 15 s,
30 s, 1 min, 5 min, 15 min, 30 min und ab zehn Fehlern 60 min. Zaehler und
Sperrstufe bleiben ueber Neustarts erhalten. Ein korrekter PIN setzt den Zaehler
zurueck.

## Freigaben

- Entsperren: Geräte-PIN
- Empfangsadresse anzeigen/bestaetigen: Geräte-PIN
- Transaktion anzeigen/bestaetigen: Geräte-PIN
- Neue Wallet erstellen: separater Root-PIN

Diese Datei definiert die verbindliche Policy. Receive, PSBT und Wallet-Erstellung
werden in ihren jeweiligen Sprints technisch daran angebunden. Der Root-PIN wird
nicht vorgetaeuscht, bevor der Root-/Seed-Ablauf implementiert ist.

## Hardwaregrenze

Waveshare BSP, Display-Refresh, BUSY-Handling, GPIO-Mapping und Herstellerkomponenten
blieben unveraendert. Fuer den Produktivbetrieb sind spaeter ESP32-S3 Secure Boot,
Flash Encryption, ein Security Review und ein definiertes Recovery-Verfahren Pflicht.
