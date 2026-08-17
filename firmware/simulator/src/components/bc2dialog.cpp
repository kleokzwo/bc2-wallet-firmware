#include "bc2dialog.h"

#include "bc2header.h"
#include <QVBoxLayout>

Bc2Dialog::Bc2Dialog(const QString &title, QWidget *parent) : QDialog(parent) {
    setModal(true);
    setMinimumWidth(420);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(18);
    layout->addWidget(new Bc2Header(title));
    contentLayout_ = new QVBoxLayout;
    contentLayout_->setSpacing(14);
    layout->addLayout(contentLayout_);
}

QVBoxLayout *Bc2Dialog::contentLayout() const { return contentLayout_; }
