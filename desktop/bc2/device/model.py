from __future__ import annotations

from dataclasses import dataclass


CAPABILITY_NAMES = (
    (0x01, "USB"),
    (0x02, "NVS"),
    (0x04, "RNG"),
    (0x08, "Display"),
    (0x10, "Tasten"),
)


@dataclass(frozen=True)
class DeviceInfo:
    port: str
    info: str
    state: int | None
    capability_flags: int
    board_revision: int
    wallet_status: int = 0

    @property
    def capabilities(self) -> tuple[str, ...]:
        return tuple(
            name for bit, name in CAPABILITY_NAMES
            if self.capability_flags & bit
        )

    @property
    def capabilities_text(self) -> str:
        return ", ".join(self.capabilities) if self.capabilities else "keine"

    @property
    def info_lines(self) -> tuple[str, ...]:
        return tuple(
            line.strip()
            for line in self.info.splitlines()
            if line.strip()
        )

    @property
    def device_name(self) -> str:
        return self.info_lines[0] if self.info_lines else "BC2 Cold Wallet"

    @property
    def hardware_name(self) -> str:
        return self.info_lines[1] if len(self.info_lines) >= 2 else "Unbekannt"

    @property
    def display_name(self) -> str:
        return self.info_lines[2] if len(self.info_lines) >= 3 else "Unbekannt"

    @property
    def firmware_revision(self) -> str:
        if len(self.info_lines) >= 4:
            value = self.info_lines[3]
            return value.split("=", 1)[1] if "=" in value else value
        return str(self.board_revision)

    @property
    def wallet_ready(self) -> bool:
        return self.wallet_status == 2

    @property
    def unlocked(self) -> bool:
        return self.state in (5, 6, 7, 8)

    @property
    def locked(self) -> bool:
        return self.state in (2, 3, 4)

    @property
    def setup_required(self) -> bool:
        return self.state == 1 and not self.wallet_ready
