#include "bc2button.h"

#include <QStyle>

Bc2Button::Bc2Button(const QString &text, Style style, QWidget *parent)
    : QPushButton(text, parent), fullText_(text), compactText_(text) {
    setCursor(Qt::PointingHandCursor);
    setStyle(style);
}

void Bc2Button::setStyle(Style style) {
    switch (style) {
    case Style::Primary:
        setObjectName(QStringLiteral("primary"));
        break;
    case Style::Secondary:
        setObjectName(QStringLiteral("secondary"));
        break;
    case Style::Navigation:
        setObjectName(QStringLiteral("nav"));
        break;
    case Style::Danger:
        setObjectName(QStringLiteral("danger"));
        break;
    }
    refreshStyle();
}

void Bc2Button::setActive(bool active) {
    setProperty("active", active);
    refreshStyle();
}

void Bc2Button::setResponsiveText(const QString &fullText, const QString &compactText) {
    fullText_ = fullText;
    compactText_ = compactText;
    setAccessibleName(fullText);
    setToolTip(fullText);
    setText(compact_ ? compactText_ : fullText_);
}

void Bc2Button::setCompact(bool compact) {
    compact_ = compact;
    setText(compact_ ? compactText_ : fullText_);
}

void Bc2Button::refreshStyle() {
    style()->unpolish(this);
    style()->polish(this);
    update();
}
