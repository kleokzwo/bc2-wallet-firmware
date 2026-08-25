# BC2 Cold Wallet – User Manual

This manual explains how to operate the **BC2 Cold Wallet community release** after the firmware and desktop application have been installed.

> [!WARNING]
> This is an experimental community implementation and is not an official Bitcoin II (BC2) product. It has not yet received a complete independent security audit or 100% penetration test. Start with small BC2 amounts.

## 1. Hardware and desktop

The **hardware wallet** is the security-critical part. It stores the installed wallet and protects the seed/private keys used during normal wallet operation. Security-critical information should be verified on the hardware display.

The **desktop wallet** provides Dashboard, Receive, Send, Transactions, Device, Settings and About. It also communicates with the configured BC2 Electrum server for blockchain synchronization and transaction broadcasting.

## 2. Connect and start

1. Connect the flashed Waveshare device using a USB data cable.
2. Close `idf.py monitor`, `picocom`, or other programs using the serial port.
3. Open the `desktop` directory.
4. Start:

```bash
./run.sh
```

## 3. First start

A factory-state device has no wallet installed. Choose either **Create New Wallet** or **Recovery Wallet**.

A BC2 Cold Wallet device manages **exactly one wallet at a time**.

## 4. Create a new wallet

1. Connect the hardware and start the desktop wallet.
2. Select **Create New Wallet**.
3. Follow the hardware instructions.
4. Create an exactly **4-digit numeric PIN**.
5. The hardware generates the wallet.
6. The recovery words are displayed on the hardware.
7. Write them down offline and verify the backup.

Never photograph, upload, email, message, or share the recovery phrase.

## 5. Unlock

1. Connect the hardware.
2. Start the desktop wallet.
3. Select **Unlock Wallet**.
4. Enter the 4-digit PIN on the hardware.
5. The Dashboard opens after successful authentication.

The desktop has **no inactivity countdown or automatic logout timer**.

## 6. Dashboard

The Dashboard shows wallet information such as current balance, unconfirmed balance, recent transactions and device/network/synchronization status.

Blockchain information is obtained by the desktop through the configured BC2 Electrum connection.

## 7. Receive BC2

1. Unlock the wallet.
2. Open **Receive**.
3. Select **Request receiving address**.
4. Check the address on the **hardware display**.
5. Confirm it on the hardware.
6. The desktop then shows the confirmed address and QR code.
7. Copy/share that confirmed address with the sender.

Always verify the receiving address on the hardware.

## 8. Send BC2

1. Unlock the wallet.
2. Open **Send**.
3. Enter the recipient address.
4. Enter the amount.
5. Continue and prepare the transaction.
6. Follow the hardware instructions.
7. Verify recipient and amount on the hardware.
8. Confirm/sign only if everything is correct.
9. The desktop broadcasts the signed transaction.

If the hardware shows unexpected information, reject the transaction.

## 9. Transactions

Open **Transactions** to view the history and confirmation state associated with the current wallet.

Transaction history and balance data are blockchain information and are separate from the seed/private keys protected by the hardware.

## 10. Lock / finish

When finished:

1. Use **Lock Wallet**.
2. Close the desktop application if no longer needed.
3. Disconnect the hardware when it is not in use.

There is no automatic inactivity countdown.

## 11. Recovery

The current implementation uses desktop-assisted recovery.

1. Open **Recovery Wallet** when recovery is available.
2. Choose 12 or 24 words.
3. Enter the recovery phrase in the desktop recovery interface.
4. The desktop validates it and transfers it to the hardware for restoration.
5. Follow the hardware instructions.
6. If required, create a new 4-digit PIN.
7. Verify the recovered wallet before using significant funds.

During recovery the computer can see the words while they are entered. Use a trusted computer. The desktop must not persist or log the phrase.

Never recover while screen sharing, streaming, or using remote-access software.

## 12. Wrong PIN / lockdown

The device PIN is exactly **4 numeric digits**.

The hardware tracks failed attempts and shows the remaining attempts. After **three failed PIN attempts**, the protected lockdown/recovery state is entered and normal unlocking is no longer available.

Do not repeatedly guess a forgotten PIN.

## 13. Settings

### Language

Available languages:

- **English**
- **Deutsch**

English is the default. Choose the language under **Settings → Language**, save it, and restart the desktop wallet for the complete interface change.

### Network

The BC2 Electrum server can be configured under **Settings → Network**.

This server is used for blockchain synchronization/broadcast communication. It does not receive custody of the hardware seed/private keys.

## 14. Hardware not detected

1. Make sure the USB cable supports data.
2. Disconnect/reconnect the hardware.
3. Close programs occupying the serial port.
4. On Linux, check for a device such as `/dev/ttyACM0`.
5. Check serial-device permissions.
6. Restart the desktop wallet if necessary.

## 15. Network offline

1. Open **Settings → Network**.
2. Check the configured BC2 Electrum server.
3. Check the computer's network connection.
4. Retry synchronization.

Hardware USB status and desktop network status are separate.

## 16. Security checklist

- [ ] I understand this is an experimental community release.
- [ ] I started with a small BC2 amount.
- [ ] My recovery phrase is backed up offline.
- [ ] I have never photographed or uploaded my recovery phrase.
- [ ] I verify receiving addresses on the hardware.
- [ ] I verify transaction details on the hardware before signing.
- [ ] I keep my 4-digit PIN private.
- [ ] I do not install firmware from an untrusted source.
- [ ] I lock/disconnect the hardware when finished.

## 17. One device, one wallet

```text
FACTORY STATE
     |
     +-- Create Wallet --> ONE WALLET
     |
     +-- Recovery ------> ONE WALLET
```

The device manages exactly one wallet at a time. Creating/recovering another wallet replaces the wallet state managed by the device.

## 18. Final reminder

Always trust information you have **verified on the hardware** over information shown only on the computer.

For installation and firmware flashing, see the repository's main `README.md`.
