# Sprint 22 – v0.27.0 Desktop-Hardware-Receive

## Ziel

Der Empfangen-Button im Qt-Simulator sendet die aktuell sichtbare öffentliche
BC2-Adresse direkt an die Waveshare-Hardware. Ein manueller Python-Aufruf ist
nicht mehr erforderlich.

## Sicherheitsgrenze

- Der Desktop überträgt nur die öffentliche Empfangsadresse.
- Seed, private Schlüssel und PIN verlassen die Hardware nicht.
- Die Firmware prüft weiterhin BC2-Mainnet, Bech32-Prüfsumme, P2WPKH-Länge,
  entsperrten Zustand und Dashboard-Kontext.
- Die erneute Geräte-PIN-Freigabe erfolgt ausschließlich auf der Hardware.

## Umsetzung

- eigener `HardwareWalletClient` mit einer kleinen Verantwortlichkeit
- automatische Erkennung über einen BC2-Protokoll-Ping
- plattformunabhängige Portsuche für Linux, Windows und macOS
- klare Fehler für nicht gefundenes, belegtes, gesperrtes oder ablehnendes Gerät
- Qt SerialPort als offizielle plattformübergreifende serielle Abhängigkeit

## Hardwaretest

1. Firmware v0.27.0 flashen und Gerät entsperren.
2. Andere serielle Monitore schließen.
3. Im Simulator `Empfangen` öffnen.
4. `Auf Gerät verifizieren` anklicken.
5. PIN auf der Hardware eingeben und alle Adresszeichen vergleichen.
