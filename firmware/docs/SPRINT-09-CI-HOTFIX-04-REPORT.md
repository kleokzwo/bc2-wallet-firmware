# BC2 Cold Wallet v0.17.4

## Windows C17 Portability Hotfix

### Ausgangslage

Der Windows-CI-Job erreichte erstmals den eigentlichen Build. MSVC stoppte den C17-Core wegen drei plattformspezifischen Problemen:

- `strtok_r` ist unter MSVC nicht verfügbar.
- `strcpy` wird mit aktivem `/W4 /WX` als unsicher abgelehnt.
- OpenSSL 3 markiert die vorhandenen, bereits getesteten `EC_KEY`-Aufrufe als veraltet.

### Umsetzung

- Neue zentrale Headerdatei `firmware/include/bc2_compat.h`.
- Plattformneutrale Tokenisierung über `bc2_strtok`.
- Größenbegrenzte Stringkopie über `bc2_copy_string`.
- BIP39- und BIP32-Parser verwenden nur noch diese gemeinsame Schicht.
- `OPENSSL_SUPPRESS_DEPRECATED` wird zentral für den Wallet-Core gesetzt.
- MSVC bleibt weiterhin auf `/W4 /WX`; Warnungen wurden nicht global abgeschaltet.

### Sicherheitsbewertung

Die Änderungen betreffen nur Compiler- und C-Laufzeit-Kompatibilität. Kryptografische Algorithmen, Ableitungspfade, Signaturformat, Seed-Verarbeitung und USB-Sicherheitsgrenzen wurden nicht verändert.

### Prüfung

- Linux CMake-Konfiguration: erfolgreich
- C17-Wallet-Core: erfolgreich
- CLI: erfolgreich
- 13/13 Tests: erfolgreich
- Windows-Bestätigung: durch erneuten GitHub-Actions-Lauf erforderlich
