#ifndef BC2_SIMULATOR_PAGEROUTER_H
#define BC2_SIMULATOR_PAGEROUTER_H

#include <QObject>

class QStackedWidget;
class QWidget;

class PageRouter final : public QObject {
    Q_OBJECT
public:
    enum class Page {
        Dashboard,
        Receive,
        WatchOnly,
        Network,
        Transaction,
        History,
        Settings,
        About,
        Recovery,
        Backup,
        Error,
        FactoryReset,
        Lock
    };
    Q_ENUM(Page)

    explicit PageRouter(QStackedWidget *stack, QObject *parent = nullptr);

    void registerPage(Page page, QWidget *widget);
    void show(Page page);
    Page currentPage() const;

signals:
    void pageChanged(Page page);

private:
    void animatePage(QWidget *widget);

    QStackedWidget *stack_;
    Page currentPage_ = Page::Dashboard;
};

#endif
