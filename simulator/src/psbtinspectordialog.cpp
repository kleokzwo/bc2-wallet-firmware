#include "psbtinspectordialog.h"
#include "watchonlymodel.h"
extern "C" {
#include "bc2_psbt.h"
}
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <vector>
#include <cstring>

static QString bc2Amount(uint64_t sats) { return QString::number(static_cast<double>(sats) / 100000000.0, 'f', 8) + QStringLiteral(" BC2"); }

PsbtInspectorDialog::PsbtInspectorDialog(const WatchOnlyModel *model, QWidget *parent)
    : QDialog(parent), model_(model) {
    setWindowTitle(QStringLiteral("BC2 Transaktion prüfen · PSBT v0")); resize(820, 620);
    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(QStringLiteral("Die Datei wird lokal gelesen. Gebühren werden nur angezeigt, wenn jeder Input ein witness_utxo enthält. Wechselgeld wird ausschließlich gegen bekannte Watch-only-Scripts geprüft."));
    intro->setWordWrap(true); layout->addWidget(intro);
    summary_ = new QLabel(QStringLiteral("Noch keine PSBT geladen.")); summary_->setWordWrap(true); layout->addWidget(summary_);
    warning_ = new QLabel(QStringLiteral("Signieren ist in v0.7.0 weiterhin deaktiviert.")); warning_->setWordWrap(true); layout->addWidget(warning_);
    outputs_ = new QTableWidget(0, 4); outputs_->setHorizontalHeaderLabels({QStringLiteral("Typ"), QStringLiteral("Adresse / Script"), QStringLiteral("Betrag"), QStringLiteral("Prüfung")});
    outputs_->horizontalHeader()->setStretchLastSection(true); outputs_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); layout->addWidget(outputs_);
    auto *button = new QPushButton(QStringLiteral("PSBT-Datei auswählen")); connect(button, &QPushButton::clicked, this, &PsbtInspectorDialog::openFile); layout->addWidget(button);
}
void PsbtInspectorDialog::openFile() {
    const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("PSBT auswählen"), {}, QStringLiteral("PSBT (*.psbt);;Alle Dateien (*)")); if (path.isEmpty()) return;
    QFile file(path); if (!file.open(QIODevice::ReadOnly)) { warning_->setText(QStringLiteral("Datei konnte nicht geöffnet werden.")); return; }
    const QByteArray data = file.readAll(); std::vector<bc2_owned_script> owned;
    if (model_ != nullptr) for (const WatchAddress &a : model_->addresses()) { if (a.scriptPubKey.size() > static_cast<int>(BC2_PSBT_MAX_SCRIPT_SIZE)) continue; bc2_owned_script s{}; s.length=static_cast<size_t>(a.scriptPubKey.size()); s.is_change=a.change?1:0; std::memcpy(s.bytes,a.scriptPubKey.constData(),s.length); owned.push_back(s); }
    bc2_psbt_summary review{}; const bc2_psbt_status st=bc2_psbt_review(reinterpret_cast<const uint8_t*>(data.constData()),static_cast<size_t>(data.size()),owned.data(),owned.size(),"bc2",&review);
    summary_->setText(QStringLiteral("Status: %1\nInputs: %2 · Outputs: %3\nGesamteingang: %4\nGesamtausgang: %5\nExterne Empfänger: %6\nVerifiziertes Wechselgeld: %7\nGebühr: %8")
        .arg(QString::fromLatin1(bc2_psbt_status_message(st))).arg(review.input_count).arg(review.output_count)
        .arg(review.all_input_amounts_known?bc2Amount(review.total_input_amount):QStringLiteral("unbekannt"))
        .arg(bc2Amount(review.total_output_amount)).arg(bc2Amount(review.external_output_amount)).arg(bc2Amount(review.change_amount))
        .arg(review.fee_known?bc2Amount(review.fee_amount):QStringLiteral("nicht berechenbar")));
    outputs_->setRowCount(static_cast<int>(review.output_count));
    for (unsigned int i=0;i<review.output_count;++i) { const auto &o=review.outputs[i]; const QString type=o.change?QStringLiteral("Wechselgeld"):(o.owned?QStringLiteral("Eigene Adresse"):QStringLiteral("Empfänger")); QString target=o.address[0]?QString::fromLatin1(o.address):QString::fromLatin1(QByteArray(reinterpret_cast<const char*>(o.script),static_cast<int>(o.script_length)).toHex());
        outputs_->setItem(static_cast<int>(i),0,new QTableWidgetItem(type)); outputs_->setItem(static_cast<int>(i),1,new QTableWidgetItem(target)); outputs_->setItem(static_cast<int>(i),2,new QTableWidgetItem(bc2Amount(o.amount))); outputs_->setItem(static_cast<int>(i),3,new QTableWidgetItem(o.change?QStringLiteral("Watch-only Script bestätigt"):QStringLiteral("vollständig anzeigen"))); }
    const bool safeToReview=(st==BC2_PSBT_OK&&review.fee_known&&review.change_verified); warning_->setText(safeToReview?QStringLiteral("Beträge, Gebühr und bekannte Wechselgeld-Scripts wurden vollständig geprüft. Signieren bleibt deaktiviert."):QStringLiteral("NICHT FREIGEBEN: unvollständige oder nicht unterstützte PSBT-Daten. Signieren bleibt deaktiviert."));
}
