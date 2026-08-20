# BC2 Cold Wallet Desktop – Phase 1 Wallet-ID Contract

- Added protocol constant `CMD_GET_WALLET_ID = 0x48`.
- Added `BC2DeviceClient.get_wallet_id()`.
- Added `device.discovery.get_wallet_id(port_name)`.
- Wallet IDs are represented on desktop as exactly 32 lowercase hexadecimal characters (128 bits).
- A locked/not-authenticated device returns `None`.
- No cache, WalletContext or UI integration is introduced in this phase.
