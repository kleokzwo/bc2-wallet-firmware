#ifndef BC2_EPAPERWIDGET_H
#define BC2_EPAPERWIDGET_H
#include <QWidget>
class EpaperWidget final:public QWidget{public: explicit EpaperWidget(QWidget*parent=nullptr); void setTitle(const QString&); void setBody(const QString&); void setFooter(const QString&); void setFullRefreshIndicator(bool);
protected:void paintEvent(QPaintEvent*) override;
private:QString title_,body_,footer_; bool fullRefresh_=false;
};
#endif
