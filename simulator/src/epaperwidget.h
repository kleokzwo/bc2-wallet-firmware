#ifndef BC2_EPAPERWIDGET_H
#define BC2_EPAPERWIDGET_H
#include <QWidget>
class EpaperWidget final:public QWidget{Q_OBJECT
public: explicit EpaperWidget(QWidget*parent=nullptr); void setTitle(const QString&); void setBody(const QString&); void setFooter(const QString&);
protected:void paintEvent(QPaintEvent*) override;
private:QString title_,body_,footer_;
};
#endif
