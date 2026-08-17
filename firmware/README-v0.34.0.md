# BC2 Firmware v0.35.0

Dieser Sprint implementiert den ersten vollständigen produktnahen Cold-Wallet-
Receive-Flow.

## Ablauf

1. Python fordert eine neue Empfangsadresse an.
2. ESP32-S3 lädt den verschlüsselten Wallet-Seed.
3. Die Adresse wird auf der Hardware aus `m/84'/4541509'/0'/0/index` abgeleitet.
4. Das Gerät verlangt die 4-stellige PIN.
5. Die vollständige Adresse erscheint auf dem E-Paper.
6. Beide Tasten bestätigen; BOOT/Back bricht ab.
7. Erst nach Bestätigung wird der Receive-Index gespeichert.
8. Erst danach wird ausschließlich die öffentliche Adresse an Python übertragen.

## Build

```bash
source ~/esp/esp-idf/export.sh
./scripts/build-hardware.sh
./scripts/flash-hardware.sh
```
