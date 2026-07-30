#ifndef BC2_PSBTINSPECTORDIALOG_H
#define BC2_PSBTINSPECTORDIALOG_H
#include <QDialog>
class QLabel;
class PsbtInspectorDialog final:public QDialog{Q_OBJECT
public:explicit PsbtInspectorDialog(QWidget*parent=nullptr);
private slots:void openFile();
private:QLabel*result_;
};
#endif
