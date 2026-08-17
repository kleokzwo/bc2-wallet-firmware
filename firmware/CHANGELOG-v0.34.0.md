# BC2 Cold Wallet Firmware v0.35.0

## Receive – Hardware Owned

- Empfangsadresse wird aus dem verschlüsselten Seed direkt auf dem ESP32-S3 abgeleitet.
- BIP39 Seed -> BIP32 -> m/84'/4541509'/0'/0/index.
- Private Key / Seed / Mnemonic werden niemals an den Desktop übertragen.
- Receive-Index wird dauerhaft in NVS gespeichert.
- Index wird erst nach physischer Hardware-Bestätigung erhöht.
- Desktop erhält die Adresse erst nach APPROVED.
- 4-stellige PIN ist vor der Adressanzeige auf der Hardware erforderlich.
- Abbruch während PIN-Freigabe und Adressprüfung wird sauber behandelt.
- Neues USB-Protokoll:
  - BEGIN_RECEIVE_ADDRESS (0x42)
  - GET_RECEIVE_RESULT (0x43)
- ESP32-S3 Kryptografie-Backend für BIP32/secp256k1 über mbedTLS ergänzt.
- OpenSSL bleibt Backend für Desktop/Host-Tests.

## Tests

Host/Core: 21/21 PASS.

Der finale ESP-IDF Target-Build muss auf der Entwicklungsmaschine mit ESP-IDF
und dem Waveshare BSP ausgeführt werden.
