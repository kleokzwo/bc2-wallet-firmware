# BC2 Cold Wallet v0.8.0 – Hardware-Abstraktionsschicht

## Ziel

v0.8.0 trennt sicherheitskritische Wallet-Logik von konkreter Hardware. Der C17-Core kennt keine Qt-, Linux- oder Mikrocontroller-API. Stattdessen verwendet er eine klar begrenzte Hardware-Abstraktionsschicht (HAL).

## Verbindliche Schnittstellen

1. **E-Paper-Display** – Übergabe eines festen 296×128-Frames mit Titel, Inhalt, Fußzeile und Full-Refresh-Hinweis.
2. **Tasten** – diskrete Ereignisse für links, rechts, bestätigen und zurück; inklusive Zeitstempel.
3. **Zeitquelle** – monotone Millisekundenquelle für Sperrzeiten und Sitzungsablauf.
4. **Zufallsquelle** – ausschließlich Plattformadapter; die Qualität muss auf Zielhardware separat geprüft werden.
5. **Nichtflüchtiger Speicher** – Schlüssel/Wert-Schnittstelle mit klaren Größen- und Fehlergrenzen.
6. **USB-Transport** – begrenzte Nachrichten bis 4096 Byte; kein direkter Zugriff auf Seed oder Private Keys.

## Simulator-Backend

Der Qt-Simulator implementiert dieselben Schnittstellen mit:

- E-Paper-Vorschau im vorhandenen Widget,
- Ereigniswarteschlange für virtuelle Tasten,
- Systemzeit,
- Qt-Systemzufall,
- flüchtigem In-Memory-Speicher,
- USB-Loopback für Entwicklungszwecke.

Der Simulator-Speicher und USB-Loopback sind ausdrücklich keine Sicherheitsimplementierung.

## Sicherheitsgrenzen

- Die HAL enthält keine Wallet- oder Kryptologik.
- Fehler werden explizit zurückgegeben; es gibt keine stillen Fallbacks.
- E-Paper-Sicherheitsseiten verlangen einen Full Refresh.
- Eingaben und USB-Nachrichten sind begrenzt.
- Sichere Speicherung, echter TRNG, Secure Element und Manipulationsschutz bleiben offen.

## Nächster Schritt v0.9.0

- konkrete Zielhardware auswählen und Pin-/Busbelegung dokumentieren,
- echtes E-Paper-Treiberbackend,
- physische Tasten und Debouncing,
- USB-HID-Protokoll mit Framing,
- persistenter Speicheradapter,
- Hardware-in-the-loop-Tests.
