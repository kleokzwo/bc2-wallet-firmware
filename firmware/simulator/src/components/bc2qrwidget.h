#ifndef BC2_SIMULATOR_BC2QRWIDGET_H
#define BC2_SIMULATOR_BC2QRWIDGET_H

#include <QWidget>

class QPaintEvent;

class Bc2QrWidget final : public QWidget {
public:
    explicit Bc2QrWidget(QWidget *parent = nullptr);
    void setPayload(const QString &payload);
    QString payload() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString payload_;
};

#endif
