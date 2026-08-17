#ifndef BC2_SIMULATOR_BC2LOADINGWIDGET_H
#define BC2_SIMULATOR_BC2LOADINGWIDGET_H

#include <QWidget>

class QLabel;
class QProgressBar;

class Bc2LoadingWidget final : public QWidget {
public:
    explicit Bc2LoadingWidget(QWidget *parent = nullptr);
    void setMessage(const QString &message);
    void setProgress(int completed, int total);
    void setBusy(bool busy);

private:
    QLabel *messageLabel_ = nullptr;
    QProgressBar *progressBar_ = nullptr;
};

#endif
