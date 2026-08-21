from __future__ import annotations


class WalletContext:
    """In-memory identity of the currently authenticated hardware wallet.

    This class deliberately owns no cache, settings or device I/O.  It only
    records which wallet identity has been authenticated for the current
    desktop session.
    """

    def __init__(self) -> None:
        self._active_wallet_id: str | None = None

    def activate(self, wallet_id: str) -> None:
        normalized = self._normalize(wallet_id)
        self._active_wallet_id = normalized

    def deactivate(self) -> None:
        self._active_wallet_id = None

    def active_wallet_id(self) -> str | None:
        return self._active_wallet_id

    @staticmethod
    def _normalize(wallet_id: str) -> str:
        if not isinstance(wallet_id, str):
            raise TypeError("wallet_id must be a string")
        value = wallet_id.strip().lower()
        if len(value) != 32:
            raise ValueError("wallet_id must contain exactly 32 hex characters")
        try:
            bytes.fromhex(value)
        except ValueError as exc:
            raise ValueError("wallet_id must contain only hexadecimal characters") from exc
        return value
