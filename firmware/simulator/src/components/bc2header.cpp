#include "bc2header.h"

#include <QLabel>
#include <QVBoxLayout>

Bc2Header::Bc2Header(const QString &title, const QString &subtitle, QWidget *parent)
    : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    titleLabel_ = new QLabel(title);
    titleLabel_->setObjectName(QStringLiteral("pageTitle"));
    titleLabel_->setWordWrap(true);
    layout->addWidget(titleLabel_);

    subtitleLabel_ = new QLabel(subtitle);
    subtitleLabel_->setObjectName(QStringLiteral("muted"));
    subtitleLabel_->setWordWrap(true);
    subtitleLabel_->setVisible(!subtitle.isEmpty());
    layout->addWidget(subtitleLabel_);
}

QLabel *Bc2Header::titleLabel() const { return titleLabel_; }
QLabel *Bc2Header::subtitleLabel() const { return subtitleLabel_; }
