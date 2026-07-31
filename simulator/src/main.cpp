#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("BC2 Cold Wallet Simulator");
    QApplication::setOrganizationName("BC2 Cold Wallet");
    QApplication::setApplicationVersion("0.17.4");

    MainWindow window;
    window.show();
    return app.exec();
}
