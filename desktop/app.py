from __future__ import annotations

import sys
from pathlib import Path

from PySide6.QtGui import QIcon
from PySide6.QtWidgets import QApplication

from bc2.ui.main_window import MainWindow


def main() -> int:
    app = QApplication(sys.argv)
    app.setApplicationName("BC2 Cold Wallet")
    app.setApplicationVersion("0.35.0")

    logo = Path(__file__).resolve().parent / "assets" / "bc2-logo.png"
    if logo.exists():
        app.setWindowIcon(QIcon(str(logo)))

    window = MainWindow()
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
