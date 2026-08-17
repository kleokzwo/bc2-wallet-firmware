#include "bc2loadingwidget.h"

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

Bc2LoadingWidget::Bc2LoadingWidget(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    messageLabel_ = new QLabel;
    messageLabel_->setObjectName(QStringLiteral("muted"));
    progressBar_ = new QProgressBar;
    progressBar_->setTextVisible(true);
    layout->addWidget(messageLabel_);
    layout->addWidget(progressBar_);
    setProgress(0, 1);
}

void Bc2LoadingWidget::setMessage(const QString &message) { messageLabel_->setText(message); }

void Bc2LoadingWidget::setProgress(int completed, int total) {
    const int safeTotal = total > 0 ? total : 1;
    progressBar_->setRange(0, safeTotal);
    progressBar_->setValue(qBound(0, completed, safeTotal));
}

void Bc2LoadingWidget::setBusy(bool busy) {
    if (busy) progressBar_->setRange(0, 0);
    else if (progressBar_->maximum() == 0) setProgress(0, 1);
}
