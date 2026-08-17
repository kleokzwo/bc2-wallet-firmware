#include "bc2card.h"

Bc2Card::Bc2Card(QWidget *parent) : QFrame(parent) {
    setObjectName(QStringLiteral("card"));
    setFrameShape(QFrame::NoFrame);
}
