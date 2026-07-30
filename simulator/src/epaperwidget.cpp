#include "epaperwidget.h"
#include <QPainter>
EpaperWidget::EpaperWidget(QWidget*p):QWidget(p){setMinimumSize(296,128);setMaximumHeight(220);}
void EpaperWidget::setTitle(const QString&s){title_=s;update();} void EpaperWidget::setBody(const QString&s){body_=s;update();} void EpaperWidget::setFooter(const QString&s){footer_=s;update();}
void EpaperWidget::paintEvent(QPaintEvent*){QPainter p(this);p.fillRect(rect(),Qt::white);p.setPen(Qt::black);p.drawRect(rect().adjusted(0,0,-1,-1));QFont f=p.font();f.setBold(true);f.setPointSize(12);p.setFont(f);p.drawText(QRect(14,10,width()-28,28),Qt::AlignLeft|Qt::AlignVCenter,title_);f.setBold(false);f.setPointSize(10);p.setFont(f);p.drawText(QRect(14,42,width()-28,height()-78),Qt::TextWordWrap|Qt::AlignCenter,body_);f.setPointSize(8);p.setFont(f);p.drawText(QRect(14,height()-28,width()-28,18),Qt::AlignCenter,footer_);}
