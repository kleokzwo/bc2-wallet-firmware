from __future__ import annotations

from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QComboBox, QDialog, QHBoxLayout, QLabel, QMessageBox,
    QPushButton, QTextEdit, QVBoxLayout,
)

from bc2.bip39 import recovery_fingerprint, validate_mnemonic


class RecoveryDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Recovery Wallet")
        self.setModal(True)
        self.setMinimumWidth(650)
        self._mnemonic: str | None = None

        layout = QVBoxLayout(self)
        layout.setContentsMargins(28, 24, 28, 24)
        layout.setSpacing(14)

        title = QLabel("Wallet wiederherstellen")
        title.setStyleSheet("font-size: 22px; font-weight: 800;")
        layout.addWidget(title)

        info = QLabel(
            "Gib deine bestehende BIP39-Recovery-Phrase ein. Sie wird nur für diesen "
            "Recovery-Vorgang im Arbeitsspeicher gehalten, nicht gespeichert und nicht geloggt."
        )
        info.setWordWrap(True)
        layout.addWidget(info)

        row = QHBoxLayout()
        row.addWidget(QLabel("Anzahl Wörter:"))
        self._count = QComboBox()
        self._count.addItems(["12", "24"])
        row.addWidget(self._count)
        row.addStretch()
        layout.addLayout(row)

        self._input = QTextEdit()
        self._input.setPlaceholderText("Recovery-Wörter in der richtigen Reihenfolge eingeben …")
        self._input.setMinimumHeight(180)
        self._input.setAcceptRichText(False)
        self._input.textChanged.connect(self._update_status)
        layout.addWidget(self._input)

        self._status = QLabel("0 Wörter eingegeben")
        self._status.setWordWrap(True)
        layout.addWidget(self._status)

        warning = QLabel(
            "Sicherheitshinweis: Bei Desktop-Recovery sieht dieser Computer die Recovery-Phrase "
            "während der Eingabe. Verwende dafür nur einen vertrauenswürdigen, möglichst offline betriebenen PC."
        )
        warning.setWordWrap(True)
        warning.setStyleSheet("color: #A65A00; background: #FFF4E7; padding: 10px; border-radius: 6px;")
        layout.addWidget(warning)

        buttons = QHBoxLayout()
        cancel = QPushButton("Abbrechen")
        cancel.clicked.connect(self.reject)
        self._submit = QPushButton("Auf Hardware übertragen")
        self._submit.clicked.connect(self._validate_and_accept)
        self._submit.setDefault(True)
        buttons.addStretch()
        buttons.addWidget(cancel)
        buttons.addWidget(self._submit)
        layout.addLayout(buttons)

    @property
    def mnemonic(self) -> str | None:
        return self._mnemonic

    def _update_status(self) -> None:
        count = len(self._input.toPlainText().split())
        self._status.setText(f"{count} Wörter eingegeben")

    def _validate_and_accept(self) -> None:
        expected = int(self._count.currentText())
        ok, message, normalized = validate_mnemonic(self._input.toPlainText(), expected)
        if not ok:
            QMessageBox.warning(self, "Recovery-Phrase ungültig", message)
            return
        fp = recovery_fingerprint(normalized)
        choice = QMessageBox.question(
            self,
            "Recovery bestätigen",
            f"{message}\n\nRecovery-Fingerprint: {fp}\n\n"
            "Die Phrase wird jetzt über USB an die BC2 Hardware Wallet übertragen. Fortfahren?",
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.No,
        )
        if choice != QMessageBox.Yes:
            return
        self._mnemonic = normalized
        self._input.clear()
        self.accept()
