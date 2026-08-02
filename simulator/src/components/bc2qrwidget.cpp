#include "bc2qrwidget.h"

#include "../receiveqrcode.h"

#include <QPainter>
#include <algorithm>
#include <memory>

Bc2QrWidget::Bc2QrWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(220, 220);
    setAccessibleName(QStringLiteral("Scanbarer Empfangs-QR-Code"));
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
    painter.fillRect(rect(), Qt::white);
    if (payload_.isEmpty()) return;

    try {
        const ReceiveQrCode qr(payload_.toStdString());
        constexpr int quietZone = 4;
        const int modules = qr.size() + quietZone * 2;
        const int scale = std::max(1, std::min(width(), height()) / modules);
        const int renderedSize = modules * scale;
        const int left = (width() - renderedSize) / 2;
        const int top = (height() - renderedSize) / 2;
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        for (int y = 0; y < qr.size(); ++y) {
            for (int x = 0; x < qr.size(); ++x) {
                if (qr.module(x, y)) {
                    painter.drawRect(left + (x + quietZone) * scale,
                                     top + (y + quietZone) * scale,
                                     scale, scale);
                }
            }
        }
    } catch (const std::exception &) {
        painter.setPen(Qt::black);
        painter.drawText(rect().adjusted(16, 16, -16, -16), Qt::AlignCenter | Qt::TextWordWrap,
                         QStringLiteral("QR-Code konnte nicht erstellt werden."));
    }
}
