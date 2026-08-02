# Verbindlicher Entwicklungsplan – Sprint 10 / v0.19.0

## Ziel
Die Receive-Seite wird zu einem praktisch nutzbaren Watch-only-Empfangsablauf ausgebaut.

## Verbindlicher Umfang
- scanbarer, lokal erzeugter QR-Code der öffentlichen BC2-Adresse
- vollständig auswählbare Empfangsadresse und Ableitungspfad
- Kopierfunktion und Explorer-Link
- vorbereitete Geräteverifikation ohne echte Hardwarefreigabe
- lokale, nicht sicherheitskritische Adresslabels
- konfigurierbares Gap-Limit mit sicherer Begrenzung neuer unbenutzter Adressen
- keine Seeds, privaten Schlüssel oder Signierung im Desktop

## Architektur
Der QR-Encoder ist eine kleine, Qt-unabhängige C++17-Komponente. Wallet-Ableitung bleibt im gemeinsamen C17-Core. Desktop-Einstellungen speichern ausschließlich Labels, Gap-Limit, Darstellung und Netzwerkdaten.

## Abnahmekriterien
- Host-Build erfolgreich
- bestehende 13 Core-Tests erfolgreich
- zusätzlicher QR-Test erfolgreich
- QR-Code extern maschinell dekodierbar
- README, CHANGELOG, Testnachweis und Sprint-Bericht aktualisiert
