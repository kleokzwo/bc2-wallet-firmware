#include "bc2statusbar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>

Bc2StatusBar::Bc2StatusBar(const QString &text, QWidget *parent) : QFrame(parent) {
    setObjectName(QStringLiteral("statusBar"));
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 10, 14, 10);
    label_ = new QLabel(text);
    label_->setWordWrap(true);
    layout->addWidget(label_);
    setState(State::Neutral);
}

void Bc2StatusBar::setText(const QString &text) { label_->setText(text); }

void Bc2StatusBar::setState(State state) {
    const char *name = "statusNeutral";
    if (state == State::Success) name = "statusSuccess";
    if (state == State::Error) name = "statusError";
    if (state == State::Warning) name = "statusWarning";
    label_->setObjectName(QString::fromLatin1(name));
    label_->style()->unpolish(label_);
    label_->style()->polish(label_);
}
