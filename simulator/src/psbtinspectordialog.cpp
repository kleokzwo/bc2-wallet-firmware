#include "psbtinspectordialog.h"
extern "C" {
#include "bc2_psbt.h"
}
#include <QFile><QFileDialog><QLabel><QPushButton><QVBoxLayout>
PsbtInspectorDialog::PsbtInspectorDialog(QWidget*p):QDialog(p){setWindowTitle("PSBT-Prüfung · nur Struktur");resize(520,260);auto*l=new QVBoxLayout(this);l->addWidget(new QLabel("Diese Version prüft PSBT-Magic, Größenlimits und die globale Map. Sie signiert nicht."));result_=new QLabel("Noch keine Datei geladen.");result_->setWordWrap(true);l->addWidget(result_);auto*b=new QPushButton("PSBT-Datei auswählen");connect(b,&QPushButton::clicked,this,&PsbtInspectorDialog::openFile);l->addWidget(b);}
void PsbtInspectorDialog::openFile(){const auto path=QFileDialog::getOpenFileName(this,"PSBT auswählen",{},"PSBT (*.psbt);;Alle Dateien (*)");if(path.isEmpty())return;QFile f(path);if(!f.open(QIODevice::ReadOnly)){result_->setText("Datei konnte nicht geöffnet werden.");return;}const QByteArray d=f.readAll();bc2_psbt_summary s{};const auto st=bc2_psbt_inspect(reinterpret_cast<const uint8_t*>(d.constData()),static_cast<size_t>(d.size()),&s);result_->setText(QString("Status: %1\nGröße: %2 Bytes\nGlobale Einträge: %3\nUnsigned Transaction vorhanden: %4\n\nKeine Signaturfreigabe in v0.5.0.").arg(QString::fromLatin1(bc2_psbt_status_message(st))).arg(s.total_size).arg(s.global_key_value_pairs).arg(s.contains_unsigned_transaction?"Ja":"Nein"));}
