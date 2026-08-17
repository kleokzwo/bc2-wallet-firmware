# Sprint 23 – v0.28.0 Hardware-Transaktionsprüfung

## Ziel

Eine vollständig lokal geprüfte PSBT-Zusammenfassung wird vom Qt-Simulator an
die Waveshare-Hardware übertragen und dort nach erneuter PIN-Eingabe vollständig
angezeigt und physisch bestätigt oder abgelehnt.

## Abnahme

- PSBT enthält genau einen externen BC2-Mainnet-P2WPKH-Empfänger.
- Alle Inputbeträge und die Gebühr sind bekannt.
- Wechselgeld wurde gegen bekannte Watch-only-Scripts geprüft.
- Gerät ist entsperrt und im Dashboard.
- Hardware zeigt Empfängeradresse, Betrag, Gebühr und Wechselgeld.
- Bestätigung erfolgt ausschließlich mit den Hardwaretasten.
- Seed, Private Key und PIN verlassen das Gerät niemals.

## Sicherheitsgrenze

v0.28.0 überträgt eine versionierte, vom Desktop erzeugte Zusammenfassung und
nicht die vollständige PSBT. Es wird weder signiert noch gesendet. Dieser Flow
ist eine UI-/Transportstufe und darf nicht als Grundlage einer späteren
Signaturfreigabe wiederverwendet werden, bis die Hardware die vollständige PSBT
selbst parst und gegen ihre eigene Wallet-Policy prüft.

## Teststatus

Der geänderte C17-Geräte-Service-Test läuft mit GCC 13.3.0 und
`-Wall -Wextra -Wpedantic -Werror` erfolgreich. Der vollständige CMake-/Qt- und
ESP-IDF-Build muss auf dem Entwicklungsrechner erfolgen, da diese Pakete in der
Erstellungsumgebung nicht installiert sind.
