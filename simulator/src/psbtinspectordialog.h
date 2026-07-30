#ifndef BC2_PSBTINSPECTORDIALOG_H
#define BC2_PSBTINSPECTORDIALOG_H
#include <QDialog>
class QLabel; class QTableWidget; class WatchOnlyModel;
class PsbtInspectorDialog final : public QDialog { public: explicit PsbtInspectorDialog(const WatchOnlyModel *model, QWidget *parent=nullptr);
private: void openFile();
private: const WatchOnlyModel *model_; QLabel *summary_; QLabel *warning_; QTableWidget *outputs_;
};
#endif
