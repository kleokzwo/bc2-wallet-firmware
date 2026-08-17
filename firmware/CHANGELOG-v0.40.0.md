# v0.41.0

- Fixed ESP32-S3 secp256k1 public-key multiplication by supplying an RNG callback to mbedTLS.
  This is required by the hardware implementation and was the likely cause of receive-address derivation failing while host tests passed.
- Added USB wallet-lock command used by Desktop logout.
- Logout only changes the device session to LOCKED; seed, wallet and PIN remain stored.
- Receive derivation failures are reported separately from physical user rejection.
