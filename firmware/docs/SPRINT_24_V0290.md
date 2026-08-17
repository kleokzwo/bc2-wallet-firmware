# Sprint 24 – v0.29.7 Verbindliche Hardwareentscheidung

## Ziel

Der Desktop darf das Annehmen einer USB-Anfrage nicht mehr mit der physischen
Transaktionsbestätigung verwechseln. Eine offene Prüfung besitzt genau einen
Ergebniszustand: ausstehend, bestätigt oder abgelehnt.

## Ablauf

1. Simulator überträgt die geprüfte Transaktionszusammenfassung.
2. Gerät nimmt genau eine Prüfung an und meldet `pending`.
3. Geräte-PIN wird ausschließlich auf der Hardware eingegeben.
4. Hardware zeigt Empfänger, Betrag, Gebühr und Wechselgeld.
5. Beide Tasten bestätigen; BOOT lehnt ab.
6. Simulator fragt den Ergebnisstatus ab und zeigt erst danach das Endergebnis.

## Abnahme

- Kein Erfolgstext vor der physischen Bestätigung.
- Ablehnung wird eindeutig im Simulator angezeigt.
- Keine zweite Anfrage während einer offenen Entscheidung.
- USB-Abbruch und 120-Sekunden-Timeout gelten nicht als Bestätigung.
- Keine Signierung und kein Broadcast.
- Waveshare-BSP, Displaytreiber und GPIO-Mapping bleiben unverändert.

## Sicherheitsgrenze

Die Hardware verarbeitet weiterhin die begrenzte Zusammenfassung aus v0.28.0,
nicht die vollständige PSBT. Die Ergebnisbindung ist eine notwendige
Transport-/UI-Sicherheitsstufe, aber noch keine Freigabe für Signierung.

## Teststatus

Die betroffenen C17-Tests wurden mit strikten Compilerwarnungen erfolgreich
ausgeführt. CMake, Qt und ESP-IDF sind in der Erstellungsumgebung nicht
installiert; vollständiger Build und Hardware-Abnahme erfolgen auf Kali Linux
und dem Waveshare ESP32-S3.
