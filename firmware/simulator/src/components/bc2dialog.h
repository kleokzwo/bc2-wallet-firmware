#ifndef BC2_SIMULATOR_BC2DIALOG_H
#define BC2_SIMULATOR_BC2DIALOG_H

#include <QDialog>

class QVBoxLayout;

class Bc2Dialog : public QDialog {
public:
    explicit Bc2Dialog(const QString &title, QWidget *parent = nullptr);
    QVBoxLayout *contentLayout() const;

private:
    QVBoxLayout *contentLayout_ = nullptr;
};

#endif
