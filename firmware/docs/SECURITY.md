# Sicherheitsgrenze

## Verwendete Kryptografie

Der Desktop-Core verwendet OpenSSL 3 für Hashes, HMAC, PBKDF2, Zufall und secp256k1/ECDSA. Es wird keine eigene Kryptografie erfunden. Die konkrete Bibliotheksstrategie für die spätere Hardware muss vor der Portierung erneut bewertet und dokumentiert werden.

## Testdaten

Tests und Simulator verwenden ausschließlich veröffentlichte deterministische Standard-Testvektoren. Keine echten Seeds, privaten Schlüssel oder PINs verwenden.

## Noch nicht produktionsreif

- PSBT-Parser und vollständige Transaktions-Policy
- Hardware-Display-, Button- und USB-Integration
- Secure Boot und Flash-Verschlüsselung
- verschlüsselte persistente Seed-Speicherung
- PIN-Retry- und Wipe-Policy
- Anti-Exfiltration Signing
- Seitenkanal- und Fault-Injection-Schutz
- reproduzierbare Firmware-Builds
- unabhängiges Audit und Hardware-Penetrationstest

Simulator-Bestätigungen sind UI-Tests und keine echte Hardware-Freigabe.
