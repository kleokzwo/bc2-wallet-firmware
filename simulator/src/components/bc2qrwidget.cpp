#include "bc2qrwidget.h"

#include <QPainter>

Bc2QrWidget::Bc2QrWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(180, 180);
    setAccessibleName(QStringLiteral("QR-Code-Vorschau"));
}

void Bc2QrWidget::setPayload(const QString &payload) {
    payload_ = payload;
    setToolTip(payload);
    update();
}

QString Bc2QrWidget::payload() const { return payload_; }

void Bc2QrWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);
    painter.setPen(Qt::black);
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    painter.drawText(rect().adjusted(18, 18, -18, -18), Qt::AlignCenter | Qt::TextWordWrap,
                     payload_.isEmpty()
                         ? QStringLiteral("QR-Code")
                         : QStringLiteral("QR-Rendering wird mit der finalen Receive-Seite aktiviert."));
}
