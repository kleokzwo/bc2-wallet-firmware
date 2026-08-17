# BC2 Cold Wallet Firmware v0.33.0

## Sprint-Ziel

Erste echte Hardware-Wallet-Erstellung auf dem ESP32-S3.

### Ablauf

1. Gerät startet ohne Wallet.
2. Falls noch keine PIN vorhanden ist: exakt 4-stellige Geräte-PIN anlegen.
3. Desktop zeigt ausschließlich den Setup-Bildschirm.
4. `Neue Wallet erstellen` sendet nur eine Setup-Anforderung an die Hardware.
5. Hardware verlangt eine physische Bestätigung mit beiden Tasten.
6. Hardware erzeugt 128 Bit kryptografische Entropie.
7. Ein hardwaregebundener 256-Bit-HMAC-Schlüssel wird einmalig in einem freien ESP32-S3-eFuse-Keyblock provisioniert.
8. Aus dem HMAC wird ein AES-256-GCM-Wrapping-Key abgeleitet.
9. Die BIP39-Entropie wird authentifiziert verschlüsselt in NVS gespeichert.
10. Die 12 BIP39-Recovery-Wörter werden nur auf dem E-Paper angezeigt, drei Wörter pro Seite.
11. Erst nach Bestätigung der vierten Recovery-Seite wird die Wallet als eingerichtet markiert.
12. Danach wechselt das Gerät in den gesperrten Zustand.

## Sicherheitsregeln

- Seed/Entropy wird niemals über USB gesendet.
- Recovery-Wörter werden niemals an den Python-Desktop gesendet.
- Private Schlüssel werden niemals an den Desktop gesendet.
- PIN bleibt exakt 4-stellig und wird nur auf der Hardware eingegeben.
- Ein vorhandener oder beschädigter Wallet-Datensatz wird nicht still überschrieben.
- Die eFuse-Provisionierung ist hardwareseitig dauerhaft und wird erst nach physischer Benutzerbestätigung ausgelöst.

## Wichtiger Produktionshinweis

v0.33.0 ist der erste echte Seed-Speicher-Sprint, aber noch **nicht für echte Geldbeträge freigegeben**.
Vor Production Release müssen mindestens Secure Boot v2 und Flash-/Firmware-Hardening abgeschlossen und auf realer Hardware geprüft werden. Dadurch wird verhindert, dass manipulierte Firmware den hardwaregebundenen HMAC-Service missbraucht.

## Build

```bash
source ~/esp/esp-idf/export.sh
./scripts/build-hardware.sh
./scripts/flash-hardware.sh
```

Nach dem Flashen den seriellen Monitor schließen, bevor der Python-Desktop den USB-Port verwendet.
