#ifndef BC2_PSBTINSPECTORDIALOG_H
#define BC2_PSBTINSPECTORDIALOG_H
#include <QDialog>
#include <QString>
class QLabel; class QPushButton; class QTableWidget; class WatchOnlyModel;
class PsbtInspectorDialog final : public QDialog { public: explicit PsbtInspectorDialog(const WatchOnlyModel *model, QWidget *parent=nullptr);
private: void openFile(); void verifyOnDevice();
private: const WatchOnlyModel *model_; QLabel *summary_; QLabel *warning_; QTableWidget *outputs_; QPushButton *verifyButton_;
    QString recipientAddress_; quint64 recipientAmount_ = 0U; quint64 changeAmount_ = 0U; quint64 feeAmount_ = 0U;
};
#endif
