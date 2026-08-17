#include "pagerouter.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QVariant>
#include <QWidget>

namespace {
constexpr const char *kPageProperty = "bc2Page";
constexpr int kTransitionDurationMs = 160;
}

PageRouter::PageRouter(QStackedWidget *stack, QObject *parent)
    : QObject(parent), stack_(stack) {
}

void PageRouter::registerPage(Page page, QWidget *widget) {
    if (stack_ == nullptr || widget == nullptr) {
        return;
    }

    widget->setProperty(kPageProperty, static_cast<int>(page));
    stack_->addWidget(widget);
}

void PageRouter::show(Page page) {
    if (stack_ == nullptr) {
        return;
    }

    for (int index = 0; index < stack_->count(); ++index) {
        QWidget *widget = stack_->widget(index);
        if (widget->property(kPageProperty).toInt() != static_cast<int>(page)) {
            continue;
        }

        const bool hasPageChanged = stack_->currentWidget() != widget;
        stack_->setCurrentWidget(widget);
        currentPage_ = page;

        if (hasPageChanged) {
            animatePage(widget);
        }

        emit pageChanged(page);
        return;
    }
}

PageRouter::Page PageRouter::currentPage() const {
    return currentPage_;
}

void PageRouter::animatePage(QWidget *widget) {
    auto *effect = new QGraphicsOpacityEffect(widget);
    effect->setOpacity(0.0);
    widget->setGraphicsEffect(effect);

    auto *animation = new QPropertyAnimation(effect, "opacity", this);
    animation->setDuration(kTransitionDurationMs);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(animation, &QPropertyAnimation::finished, widget, [widget] {
        widget->setGraphicsEffect(nullptr);
    });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}
