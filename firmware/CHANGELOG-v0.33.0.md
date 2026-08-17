# v0.33.0

- echte Wallet-Lifecycle-Erkennung statt `wallet_is_initialized = 1`
- neue USB-Kommandos `GET_WALLET_STATUS` und `BEGIN_CREATE_WALLET`
- Setup-Required wird jetzt wirklich verwendet
- 4-stellige PIN bleibt Voraussetzung
- echte 128-Bit-BIP39-Entropy auf der Hardware
- 12 Recovery-Wörter aus offizieller BIP39-English-Wordlist
- Recovery-Wörter ausschließlich auf dem E-Paper
- drei Recovery-Wörter pro Seite, vier Seiten
- Wallet wird erst nach vollständiger Backup-Bestätigung aktiviert
- hardwaregebundener HMAC-Schlüssel in read-protected eFuse-Keyblock
- AES-256-GCM für authentifizierte Seed-Entropy-Speicherung in NVS
- Wallet-Storage-Korruption führt zu Sicherheitsfehler statt Überschreiben
- bestehende USB-/Display-/Button-Basis aus v0.32.2 bleibt erhalten
- Core/Protocol Tests weiterhin 21/21 erfolgreich
