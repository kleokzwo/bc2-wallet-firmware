#ifndef BC2_SIMULATOR_BC2STATUSBAR_H
#define BC2_SIMULATOR_BC2STATUSBAR_H

#include <QFrame>

class QLabel;

class Bc2StatusBar final : public QFrame {
public:
    enum class State { Neutral, Success, Error, Warning };

    explicit Bc2StatusBar(const QString &text = {}, QWidget *parent = nullptr);
    void setText(const QString &text);
    void setState(State state);

private:
    QLabel *label_ = nullptr;
};

#endif
