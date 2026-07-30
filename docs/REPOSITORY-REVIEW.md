# Prüfung des übernommenen Repository-Stands

## Vorhanden und funktionsfähig

- CMake-Build für C17.
- OpenSSL-3-Anbindung.
- SHA-256, SHA-256d, RIPEMD-160 und HASH160.
- HMAC-SHA512 und PBKDF2-HMAC-SHA512.
- Base58, Base58Check, Hex und SegWit-Bech32/Bech32m-Encoding.
- BIP39-Validierung, 12-Wort-Erzeugung und Seed-Ableitung.
- BIP32-Masterkey, private Child-Key-Ableitung und xprv/xpub-Serialisierung.
- secp256k1-Public-Key-Erzeugung.
- P2PKH- und P2WPKH-Adressen.
- ECDSA-DER-Signatur, Low-S-Normalisierung und Verifikation.
- Secure Zero und kryptografischer Zufall.
- CLI und sechs bestehende Unit-Tests.

## Festgestellte Lücken

- Kein Qt-Simulator vorhanden.
- Kein zentraler Wallet-Service oder stabiles Wallet-Zustandsmodell.
- Netzwerkparameter waren direkt im CLI verteilt und nicht als eigene Konfiguration gekapselt.
- Kein Transaktions- oder PSBT-Parser.
- Keine Policy Engine zur vollständigen Transaktionsprüfung.
- Keine PIN-, Speicher-, USB-, Display- oder Hardware-Abstraktion.
- Keine Fuzztests, Sanitizer-Konfiguration oder statische Analyse im Repository.
- OpenSSL-secp256k1-Nutzung enthält noch als veraltet markierte APIs.
- BIP39-Wortliste wird zur Laufzeit aus einer Datei gelesen; das ist für spätere Firmware ungeeignet.
- Der vorhandene Testumfang prüft wichtige Grundvektoren, aber noch nicht ausreichend viele Negativ-, Grenz- und Interoperabilitätsfälle.

## Sicherheitsbewertung

Der Stand ist ein brauchbarer Entwicklungs-Core, aber keine produktionsreife Hardware-Wallet. Insbesondere Transaktionsprüfung, Secret Storage, PIN-Retry-Policy, Secure Boot, Firmware-Authentizität, physischer Schutz und unabhängiges Audit fehlen weiterhin.
