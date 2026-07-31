#include "walletpages.h"

#include "../components/bc2button.h"
#include "../components/bc2card.h"
#include "../components/bc2header.h"
#include "../components/bc2statusbar.h"
#include "../designtokens.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

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

TransactionPage::TransactionPage(QWidget *parent) : QWidget(parent) {
    auto *layout = pageLayout(this, QStringLiteral("Transaktion"),
                              QStringLiteral("PSBT prüfen, Beträge vergleichen und erst am Gerät bestätigen."));
    layout->addWidget(informationCard(QStringLiteral("Sicherheitsprüfung"),
        QStringLiteral("Empfänger, Betrag, Netzwerkgebühr und Wechselgeld müssen vollständig angezeigt werden. Private Schlüssel verlassen das Gerät nie.")));
    auto *button = new Bc2Button(QStringLiteral("PSBT-Prüfung öffnen"));
    connect(button, &QPushButton::clicked, this, &TransactionPage::reviewRequested);
    layout->addWidget(button, 0, Qt::AlignLeft);
    layout->addStretch();
}

HistoryPage::HistoryPage(QWidget *parent) : QWidget(parent) {
    auto *layout = pageLayout(this, QStringLiteral("Verlauf"),
                              QStringLiteral("Watch-only Transaktionsübersicht ohne private Schlüssel."));
    auto *table = new QTableWidget(0, 4);
    table->setHorizontalHeaderLabels({QStringLiteral("Status"), QStringLiteral("Datum"),
                                      QStringLiteral("Transaktion"), QStringLiteral("Betrag")});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(table, 1);
    layout->addWidget(new Bc2StatusBar(QStringLiteral("Noch keine synchronisierten Transaktionen.")));
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
    layout->addWidget(informationCard(QStringLiteral("Version 0.17.0"),
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
