# BC2 Cold Wallet – Community HOWTO

> **Experimental community release.** This project is under active development and has not been independently security-audited. Start with small amounts only. Never use a seed from another wallet for testing.

This guide explains how to prepare the supported Waveshare development board, flash the BC2 Cold Wallet firmware, install the desktop application, and start using the wallet.

## 1. What you need

- **Waveshare ESP32-S3-ePaper-1.54**, 200×200 e-paper display, supported board revision.
- A data-capable USB cable.
- A Linux PC for the documented development/flash flow.
- **ESP-IDF 5.5.0 or newer** for building and flashing the firmware.
- Python 3 and `venv` support for the desktop wallet.
- This repository checked out locally.

The BC2 Cold Wallet is designed for **USB-only operation**. Wi-Fi and Bluetooth are not part of the wallet workflow.

## 2. Get the repository

```bash
git clone <YOUR-BC2-COLD-WALLET-REPOSITORY-URL>
cd bc2-wallet-firmware-main
```

If you downloaded a ZIP instead, extract it and open a terminal in the extracted repository directory.

## 3. Build and flash the hardware firmware

First activate your ESP-IDF environment. Example:

```bash
. ~/esp/esp-idf/export.sh
```

Then build the supported Waveshare target:

```bash
cd firmware
./scripts/build-hardware.sh
```

Connect the Waveshare board by USB and flash it:

```bash
./scripts/flash-hardware.sh
```

The helper uses ESP-IDF auto-detection and opens the monitor after flashing. If you need to select a serial port explicitly, use the normal ESP-IDF workflow directly from the hardware target directory:

```bash
cd hardware/esp32s3_waveshare
idf.py build
idf.py -p /dev/ttyACM0 flash
idf.py monitor
```

Exit the ESP-IDF monitor with its normal monitor exit shortcut.

### Optional USB probe

The repository contains a public diagnostic probe. It does not require a seed, private key, or PIN:

```bash
python -m pip install pyserial
python firmware/bc2_probe.py
```

Use diagnostic tools only with test wallets and test data.

## 4. Start the desktop wallet

From the repository root:

```bash
cd desktop
chmod +x run.sh
./run.sh
```

`run.sh` creates/uses the Python virtual environment and starts the PySide6 desktop application using the repository requirements.

The desktop wallet communicates with the hardware over USB. The default BC2 Electrum endpoint can be changed in **Settings → Network**.

## 5. Choose the desktop language

The community build uses **English by default**. German is also included.

Open:

**Settings → Language**

Choose either:

- `English`
- `Deutsch`

Click **Save settings**. For safety and to avoid rebuilding active wallet screens while a session is open, the new language is applied after restarting the desktop wallet.

## 6. Create a new wallet

Use this only when the device is in factory state and does not already contain a wallet.

1. Connect the hardware by USB.
2. Start the desktop wallet.
3. Select **Create New Wallet**.
4. Follow the instructions on the hardware.
5. Create the required **4-digit PIN** on the hardware.
6. The hardware creates the wallet and displays the recovery words **on the hardware only**.
7. Write the recovery words down offline and store them securely.
8. Never photograph, upload, email, or paste a production recovery phrase into online services.

A BC2 Cold Wallet device manages **exactly one wallet at a time**.

## 7. Unlock an existing wallet

1. Connect the hardware by USB.
2. Start the desktop wallet.
3. Select **Unlock Wallet**.
4. Enter the 4-digit PIN on the hardware.
5. After successful unlock, the desktop opens the wallet dashboard.

After three failed PIN attempts the device enters its protected recovery/lockdown flow.

## 8. Recover a wallet

Recovery is intentionally different from wallet creation.

1. Start **Recover Wallet** from the desktop when recovery is available.
2. Choose 12 or 24 words.
3. Enter the BIP39 recovery phrase in the desktop recovery dialog.
4. The desktop validates and transfers it to the hardware for the recovery operation.
5. The desktop must not store or log the recovery phrase.
6. Follow the hardware instructions to complete recovery and PIN setup/verification.

During desktop-assisted recovery, the computer can see the recovery phrase while it is being entered. Use a trusted computer, preferably offline except for the minimum required wallet operation.

## 9. Receive BC2

1. Unlock the hardware wallet.
2. Open **Receive**.
3. Request a receiving address.
4. Check the address on the hardware display.
5. Confirm it on the hardware.
6. Only after hardware confirmation does the desktop show the address and QR code.

Never trust an address shown only on the computer if it has not been verified on the hardware.

## 10. Send BC2

1. Unlock the wallet.
2. Open **Send**.
3. Enter the recipient address and amount.
4. Prepare the transaction.
5. Review the transaction details on the hardware.
6. Confirm the transaction on the hardware.
7. The hardware signs the transaction.
8. The desktop broadcasts the signed transaction through the configured BC2 Electrum server.

Private keys and the seed remain on the hardware during normal wallet operation.

## 11. Lock the wallet

Use **Lock wallet** in the desktop sidebar when you are finished. The desktop wallet has **no inactivity countdown or automatic logout timer**. Disconnect the hardware when it is no longer needed.

## 12. Troubleshooting

### Hardware is not found

- Make sure the USB cable supports data, not charging only.
- Reconnect the board.
- Check whether `/dev/ttyACM0` or another serial device appears.
- Make sure your Linux user has permission to access the serial port.
- Close `idf.py monitor`, `picocom`, or any other program that may already have the serial port open.

### Desktop does not start

From `desktop/` run:

```bash
./run.sh
```

If dependencies are missing, verify that Python, `venv`, and the packages in `requirements.txt` can be installed.

### Electrum/network is offline

Open **Settings → Network** and verify the configured BC2 Electrum server. Hardware wallet functions that do not require blockchain synchronization remain separate from the network service.

## 13. Security rules for the community release

- Treat this release as **experimental**.
- Start with small BC2 amounts.
- Do not use real production seeds for development or debugging.
- Verify receiving addresses on the hardware.
- Review transaction details on the hardware before signing.
- Keep the recovery phrase offline.
- Never share the 4-digit PIN.
- Do not install modified firmware from an untrusted source.

---

# Deutsche Kurzanleitung

> **Experimenteller Community-Release.** Das Projekt befindet sich in aktiver Entwicklung und wurde noch nicht unabhängig sicherheitsauditiert. Bitte zuerst nur mit kleinen Beträgen testen.

## Voraussetzungen

Benötigt werden das unterstützte **Waveshare ESP32-S3-ePaper-1.54 (200×200)**, ein USB-Datenkabel, ESP-IDF 5.5.0 oder neuer sowie Python für die Desktop Wallet.

## Firmware flashen

```bash
. ~/esp/esp-idf/export.sh
cd firmware
./scripts/build-hardware.sh
./scripts/flash-hardware.sh
```

Alternativ:

```bash
cd firmware/hardware/esp32s3_waveshare
idf.py build
idf.py -p /dev/ttyACM0 flash
idf.py monitor
```

## Desktop Wallet starten

```bash
cd desktop
chmod +x run.sh
./run.sh
```

## Sprache wechseln

Unter **Einstellungen → Sprache** zwischen **English** und **Deutsch** auswählen und die Einstellungen speichern. Die neue Sprache wird nach einem Neustart der Desktop Wallet aktiv.

## Neue Wallet

**Create New Wallet** auswählen, auf der Hardware eine exakt 4-stellige PIN anlegen und die ausschließlich auf der Hardware angezeigten Recovery-Wörter offline sichern.

## Wallet entsperren

Hardware per USB verbinden, **Unlock Wallet** auswählen und die 4-stellige PIN auf der Hardware eingeben. Die Desktop Wallet verwendet keinen Inaktivitäts-Countdown und keinen automatischen Logout-Timer.

## BC2 empfangen

Unter **Empfangen** eine Adresse anfordern und die Adresse unbedingt auf der Hardware prüfen und bestätigen. Erst danach wird sie im Desktop angezeigt.

## BC2 senden

Empfänger und Betrag eingeben, Transaktion vorbereiten, Details auf der Hardware prüfen, dort bestätigen/signieren und anschließend über den Desktop senden.

## Recovery

Bei Recovery werden 12 oder 24 BIP39-Wörter im Desktop eingegeben, validiert und nur für den Recovery-Vorgang an die Hardware übertragen. Die Recovery-Phrase darf vom Desktop nicht gespeichert oder geloggt werden.

## Wichtig

Der Community-Release ist zum Testen gedacht. Kleine Beträge verwenden, Seed offline halten und niemals ungeprüfte Adressen oder Transaktionen nur anhand des Desktop-Displays bestätigen.
