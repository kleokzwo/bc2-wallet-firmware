#include "walletpages.h"

#include "../components/bc2button.h"
#include "../components/bc2card.h"
#include "../components/bc2header.h"
#include "../components/bc2statusbar.h"
#include "../designtokens.h"

#include <QAbstractItemView>
#include <algorithm>
#include <cstring>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

extern "C" {
#include "bc2_transaction.h"
#include "bc2_network.h"
}

namespace {
QLabel *muted(const QString &text) {
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("muted"));
    label->setWordWrap(true);
    return label;
}

QLabel *sectionTitle(const QString &text) {
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("sectionTitle"));
    label->setWordWrap(true);
    return label;
}

QVBoxLayout *pageLayout(QWidget *page, const QString &title, const QString &subtitle) {
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(DesignTokens::PageMarginWide, 36,
                               DesignTokens::PageMarginWide, 36);
    layout->setSpacing(DesignTokens::PageSpacing);
    layout->addWidget(new Bc2Header(title, subtitle));
    return layout;
}

Bc2Card *informationCard(const QString &title, const QString &body) {
    auto *card = new Bc2Card;
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(10);
    layout->addWidget(sectionTitle(title));
    layout->addWidget(muted(body));
    return card;
}
}

TransactionPage::TransactionPage(WatchOnlyModel *model, QWidget *parent)
    : QWidget(parent), model_(model) {
    auto *layout = pageLayout(this, QStringLiteral("Senden"),
                              QStringLiteral("Watch-only Transaktionsentwurf erstellen und als unsigned PSBT exportieren."));

    auto *formCard = new Bc2Card;
    auto *form = new QFormLayout(formCard);
    form->setContentsMargins(24, 22, 24, 22);
    recipientEdit_ = new QLineEdit;
    recipientEdit_->setPlaceholderText(QStringLiteral("BC2-Empfangsadresse"));
    form->addRow(QStringLiteral("Empfänger"), recipientEdit_);
    amountSpin_ = new QDoubleSpinBox;
    amountSpin_->setDecimals(8);
    amountSpin_->setRange(0.00000001, 21000000.0);
    amountSpin_->setSingleStep(0.001);
    amountSpin_->setSuffix(QStringLiteral(" BC2"));
    form->addRow(QStringLiteral("Betrag"), amountSpin_);
    feeRateSpin_ = new QSpinBox;
    feeRateSpin_->setRange(1, 1000);
    feeRateSpin_->setValue(2);
    feeRateSpin_->setSuffix(QStringLiteral(" sat/vB"));
    form->addRow(QStringLiteral("Gebührenrate"), feeRateSpin_);
    layout->addWidget(formCard);

    auto *actions = new QHBoxLayout;
    auto *createButton = new Bc2Button(QStringLiteral("Unsigned PSBT erstellen"));
    connect(createButton, &QPushButton::clicked, this, &TransactionPage::createDraft);
    actions->addWidget(createButton);
    auto *reviewButton = new Bc2Button(QStringLiteral("Vorhandene PSBT prüfen"));
    connect(reviewButton, &QPushButton::clicked, this, &TransactionPage::openExistingPsbt);
    actions->addWidget(reviewButton);
    actions->addStretch();
    layout->addLayout(actions);

    previewLabel_ = muted(QStringLiteral("Coin Selection erfolgt deterministisch aus den synchronisierten UTXOs. Es findet keine Signierung statt."));
    layout->addWidget(previewLabel_);
    layout->addWidget(informationCard(QStringLiteral("Sicherheitsgrenze"),
        QStringLiteral("Diese Seite erzeugt ausschließlich eine unsigned PSBT. Private Schlüssel und Seed werden weder benötigt noch gespeichert. Die spätere Hardware muss Empfänger, Betrag, Gebühr und Wechselgeld vollständig bestätigen.")));
    layout->addStretch();
}

void TransactionPage::createDraft() {
    if (model_ == nullptr) return;
    uint8_t recipientScript[BC2_TX_MAX_SCRIPT_SIZE] = {0};
    size_t recipientScriptLength = 0U;
    const QByteArray recipient = recipientEdit_->text().trimmed().toLatin1();
    bc2_tx_status status = bc2_address_to_script(recipient.constData(), bc2_network_mainnet(),
                                                 recipientScript, sizeof(recipientScript), &recipientScriptLength);
    if (status != BC2_TX_OK) {
        QMessageBox::warning(this, QStringLiteral("Ungültige Adresse"), QStringLiteral("Bitte eine gültige BC2-Mainnet-Adresse eingeben."));
        return;
    }

    QVector<bc2_tx_utxo> available;
    const WatchAddress *changeAddress = nullptr;
    for (const WatchAddress &address : model_->addresses()) {
        if (address.change && changeAddress == nullptr) changeAddress = &address;
        for (const WatchUtxo &utxo : address.utxos) {
            QByteArray txid = QByteArray::fromHex(utxo.txHash.toLatin1());
            if (txid.size() != 32 || utxo.value <= 0 || address.scriptPubKey.isEmpty()) continue;
            bc2_tx_utxo item{};
            std::reverse(txid.begin(), txid.end());
            memcpy(item.txid, txid.constData(), 32U);
            item.output_index = static_cast<uint32_t>(utxo.outputIndex);
            item.amount = static_cast<uint64_t>(utxo.value);
            item.script_length = qMin(static_cast<size_t>(address.scriptPubKey.size()), static_cast<size_t>(BC2_TX_MAX_SCRIPT_SIZE));
            memcpy(item.script, address.scriptPubKey.constData(), item.script_length);
            available.push_back(item);
        }
    }
    if (available.isEmpty() || changeAddress == nullptr) {
        QMessageBox::information(this, QStringLiteral("Keine UTXOs"), QStringLiteral("Wallet zuerst synchronisieren. Für den Entwurf werden bestätigte oder unbestätigte UTXOs benötigt."));
        return;
    }

    const uint64_t amount = static_cast<uint64_t>(amountSpin_->value() * 100000000.0 + 0.5);
    bc2_tx_plan plan{};
    status = bc2_transaction_plan(available.constData(), static_cast<size_t>(available.size()), amount,
                                  static_cast<uint64_t>(feeRateSpin_->value()), 546U, &plan);
    if (status != BC2_TX_OK) {
        QMessageBox::warning(this, QStringLiteral("Entwurf nicht möglich"), QString::fromLatin1(bc2_tx_status_message(status)));
        return;
    }

    uint8_t psbt[65536] = {0}; size_t psbtLength = 0U;
    status = bc2_psbt_create(available.constData(), static_cast<size_t>(available.size()), &plan,
                             recipientScript, recipientScriptLength,
                             reinterpret_cast<const uint8_t *>(changeAddress->scriptPubKey.constData()),
                             static_cast<size_t>(changeAddress->scriptPubKey.size()),
                             psbt, sizeof(psbt), &psbtLength);
    if (status != BC2_TX_OK) {
        QMessageBox::critical(this, QStringLiteral("PSBT-Fehler"), QString::fromLatin1(bc2_tx_status_message(status)));
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(this, QStringLiteral("Unsigned PSBT speichern"),
                                                           QStringLiteral("bc2-unsigned.psbt"), QStringLiteral("PSBT (*.psbt)"));
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly) || file.write(reinterpret_cast<const char *>(psbt), static_cast<qint64>(psbtLength)) != static_cast<qint64>(psbtLength)) {
        QMessageBox::critical(this, QStringLiteral("Speichern fehlgeschlagen"), QStringLiteral("Die PSBT-Datei konnte nicht vollständig gespeichert werden."));
        return;
    }
    previewLabel_->setText(QStringLiteral("%1 Eingänge · %2 BC2 Betrag · %3 BC2 Gebühr · %4 BC2 Wechselgeld · %5 vB geschätzt")
        .arg(plan.selected_input_count)
        .arg(QString::number(static_cast<double>(plan.recipient_amount) / 100000000.0, 'f', 8))
        .arg(QString::number(static_cast<double>(plan.fee_amount) / 100000000.0, 'f', 8))
        .arg(QString::number(static_cast<double>(plan.change_amount) / 100000000.0, 'f', 8))
        .arg(plan.estimated_vbytes));
    QMessageBox::information(this, QStringLiteral("PSBT erstellt"), QStringLiteral("Unsigned PSBT wurde gespeichert. Sie ist nicht signiert und noch nicht sendefähig."));
}

void TransactionPage::openExistingPsbt() {
    emit reviewRequested();
}

HistoryPage::HistoryPage(QWidget *parent) : QWidget(parent) {
    auto *layout = pageLayout(this, QStringLiteral("Verlauf"),
                              QStringLiteral("Watch-only Transaktionsübersicht ohne private Schlüssel."));
    table_ = new QTableWidget(0, 6);
    table_->setHorizontalHeaderLabels({QStringLiteral("Status"), QStringLiteral("Datum"),
                                       QStringLiteral("Uhrzeit"), QStringLiteral("TXID"),
                                       QStringLiteral("Betrag"), QStringLiteral("Bestätigungen")});
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    layout->addWidget(table_, 1);
    statusLabel_ = muted(QStringLiteral("Noch keine synchronisierten Transaktionen."));
    layout->addWidget(statusLabel_);
}

void HistoryPage::setTransactions(const QVector<WatchTransaction> &transactions, int blockHeight) {
    table_->setRowCount(transactions.size());
    for (int row = 0; row < transactions.size(); ++row) {
        const WatchTransaction &transaction = transactions.at(row);
        const int confirmations = transaction.height > 0 && blockHeight >= transaction.height
            ? blockHeight - transaction.height + 1 : 0;
        table_->setItem(row, 0, new QTableWidgetItem(confirmations > 0 ? QStringLiteral("Bestätigt") : QStringLiteral("Ausstehend")));
        table_->setItem(row, 1, new QTableWidgetItem(QStringLiteral("–")));
        table_->setItem(row, 2, new QTableWidgetItem(QStringLiteral("–")));
        table_->setItem(row, 3, new QTableWidgetItem(transaction.txHash));
        const double amount = static_cast<double>(transaction.knownAmount) / 100000000.0;
        table_->setItem(row, 4, new QTableWidgetItem(QString::number(amount, 'f', 8) + QStringLiteral(" BC2")));
        table_->setItem(row, 5, new QTableWidgetItem(QString::number(confirmations)));
    }
    statusLabel_->setText(transactions.isEmpty()
        ? QStringLiteral("Keine Transaktionen für die beobachteten Adressen gefunden.")
        : QStringLiteral("%1 öffentliche Transaktionen synchronisiert.").arg(transactions.size()));
}

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent) {
    auto *layout = pageLayout(this, QStringLiteral("Einstellungen"),
                              QStringLiteral("Darstellung und nicht sicherheitskritische Desktop-Einstellungen."));
    auto *card = new Bc2Card;
    auto *form = new QFormLayout(card);
    form->setContentsMargins(24, 22, 24, 22);
    auto *language = new QComboBox;
    language->addItems({QStringLiteral("Deutsch"), QStringLiteral("English")});
    form->addRow(QStringLiteral("Sprache"), language);
    auto *lockInfo = new QLineEdit(QStringLiteral("5 Minuten"));
    lockInfo->setReadOnly(true);
    form->addRow(QStringLiteral("Automatische Sperre"), lockInfo);
    auto *telemetry = new QCheckBox(QStringLiteral("Keine Telemetrie senden"));
    telemetry->setChecked(true);
    telemetry->setEnabled(false);
    form->addRow(QString(), telemetry);
    layout->addWidget(card);
    layout->addWidget(new Bc2StatusBar(QStringLiteral("Sicherheitskritische Einstellungen werden später ausschließlich am Gerät bestätigt.")));
    layout->addStretch();
}

AboutPage::AboutPage(QWidget *parent) : QWidget(parent) {
    auto *layout = pageLayout(this, QStringLiteral("Über BC2 Cold Wallet"),
                              QStringLiteral("Open-Source BC2-only Hardware-Wallet."));
    layout->addWidget(informationCard(QStringLiteral("Version 0.22.0"),
        QStringLiteral("Desktop-Simulator: Qt 6 · Wallet-Core: C17 · Zielhardware: Waveshare ESP32-S3 1.54\" E-Paper AIoT Development Board · Firmware: ESP-IDF.")));
    layout->addWidget(informationCard(QStringLiteral("Sicherheitsmodell"),
        QStringLiteral("Seed und private Schlüssel bleiben auf der Hardware. Empfangsadressen und Transaktionen müssen vollständig auf dem Gerät geprüft und bestätigt werden.")));
    layout->addStretch();
}

RecoveryPage::RecoveryPage(QWidget *parent) : QWidget(parent) {
    auto *layout = pageLayout(this, QStringLiteral("Wiederherstellung"),
                              QStringLiteral("Vorbereitung des späteren gerätegeführten Recovery-Ablaufs."));
    layout->addWidget(informationCard(QStringLiteral("Noch nicht aktiv"),
        QStringLiteral("Der Desktop darf niemals Recovery-Wörter entgegennehmen. Die vollständige Eingabe und Bestätigung wird ausschließlich auf der Hardware erfolgen.")));
    auto *status = new Bc2StatusBar(QStringLiteral("Keine Seed-Eingabe im Simulator."));
    status->setState(Bc2StatusBar::State::Warning);
    layout->addWidget(status);
    layout->addStretch();
}

BackupPage::BackupPage(QWidget *parent) : QWidget(parent) {
    auto *layout = pageLayout(this, QStringLiteral("Backup"),
                              QStringLiteral("Sichere, gerätegeführte Sicherung vorbereiten."));
    layout->addWidget(informationCard(QStringLiteral("Backup-Grundsätze"),
        QStringLiteral("Recovery-Wörter niemals fotografieren, digital speichern oder in den Computer eingeben. Das Gerät wird jeden Schritt lokal anzeigen und bestätigen lassen.")));
    layout->addStretch();
}

ErrorPage::ErrorPage(QWidget *parent) : QWidget(parent) {
    auto *layout = pageLayout(this, QStringLiteral("Fehler"),
                              QStringLiteral("Sicherer Fehlerzustand ohne vertrauliche Details."));
    auto *card = new Bc2Card;
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 22, 24, 22);
    cardLayout->addWidget(sectionTitle(QStringLiteral("Aktion nicht möglich")));
    messageLabel_ = muted(QStringLiteral("Es ist kein Fehler gemeldet."));
    cardLayout->addWidget(messageLabel_);
    layout->addWidget(card);
    layout->addStretch();
}

void ErrorPage::setMessage(const QString &message) {
    if (messageLabel_ != nullptr) messageLabel_->setText(message);
}

FactoryResetPage::FactoryResetPage(QWidget *parent) : QWidget(parent) {
    auto *layout = pageLayout(this, QStringLiteral("Werkseinstellungen"),
                              QStringLiteral("Gefahrenbereich – später nur mit Geräte-PIN und physischer Bestätigung."));
    layout->addWidget(informationCard(QStringLiteral("Alle Wallet-Daten löschen"),
        QStringLiteral("Ein Factory Reset wird Seed, PIN und Geräteeinstellungen unwiderruflich löschen. Diese Simulatorseite führt absichtlich keine Löschung aus.")));
    auto *button = new Bc2Button(QStringLiteral("Reset noch nicht verfügbar"), Bc2Button::Style::Danger);
    button->setEnabled(false);
    layout->addWidget(button, 0, Qt::AlignLeft);
    auto *status = new Bc2StatusBar(QStringLiteral("Hardware-Implementierung folgt in einem späteren Sprint."));
    status->setState(Bc2StatusBar::State::Warning);
    layout->addWidget(status);
    layout->addStretch();
}
