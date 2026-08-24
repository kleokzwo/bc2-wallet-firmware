# PIN and Lockdown

## For users

BC2 Cold Wallet uses an exactly four-digit device PIN.

The PIN protects normal access to an installed wallet. After three consecutive incorrect PIN attempts, the hardware enters **LOCKDOWN**. Normal PIN unlock is no longer the recovery path in this state; wallet recovery is required.

The device also locks an unlocked wallet automatically after its hardware session timeout.

### What happens after wrong PIN attempts?

- First wrong PIN: wallet remains locked.
- Second wrong PIN: wallet remains locked.
- Third wrong PIN: device enters security LOCKDOWN.

Internal hardware testing confirmed this behavior on 2026-08-24.

### What does LOCKDOWN mean?

LOCKDOWN prevents normal unlocking. It does not mean that the protocol reports the wallet as uninitialized. During the recorded test, the device reported `LOCKDOWN` while the wallet remained `INITIALIZED`.

To regain access, use the Recovery workflow with the correct recovery words and establish a new four-digit PIN as instructed by the device/application.

## For developers

Relevant states:

```text
LOCKED   = 0x02
DASHBOARD= 0x05
LOCKDOWN = 0x0A
```

The firmware default maximum failed unlock attempts is three. The desktop should react to hardware state rather than maintaining a separate authoritative PIN-attempt count.

When state is LOCKDOWN, the setup UI should expose Recovery and must not offer normal Unlock or Create New Wallet.
