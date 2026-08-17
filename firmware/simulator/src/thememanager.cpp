#include "thememanager.h"
#include "designtokens.h"

#include <QApplication>

ThemeManager::ThemeManager(QApplication *application, QObject *parent)
    : QObject(parent), application_(application) {
    setTheme(Theme::Dark);
}

ThemeManager::Theme ThemeManager::theme() const {
    return theme_;
}

QString ThemeManager::themeName() const {
    return theme_ == Theme::Dark ? QStringLiteral("Dark Mode") : QStringLiteral("Light Mode");
}

void ThemeManager::setTheme(Theme theme) {
    if (application_ == nullptr) {
        return;
    }

    theme_ = theme;
    application_->setStyleSheet(styleSheetFor(theme_));
    emit themeChanged(theme_);
}

void ThemeManager::toggleTheme() {
    setTheme(theme_ == Theme::Dark ? Theme::Light : Theme::Dark);
}

QString ThemeManager::styleSheetFor(Theme theme) const {
    const bool dark = theme == Theme::Dark;
    const QString background = dark ? QStringLiteral("#0B0E11") : QStringLiteral("#F3F5F7");
    const QString surface = dark ? QStringLiteral("#151A1F") : QStringLiteral("#FFFFFF");
    const QString sidebar = dark ? QStringLiteral("#101419") : QStringLiteral("#E8ECEF");
    const QString control = dark ? QStringLiteral("#20262C") : QStringLiteral("#F8FAFB");
    const QString input = dark ? QStringLiteral("#0F1418") : QStringLiteral("#FFFFFF");
    const QString text = dark ? QStringLiteral("#F5F7F8") : QStringLiteral("#171B1F");
    const QString muted = dark ? QStringLiteral("#929DA7") : QStringLiteral("#5D6872");
    const QString border = dark ? QStringLiteral("#2D363E") : QStringLiteral("#D0D7DC");
    const QString hover = dark ? QStringLiteral("#273038") : QStringLiteral("#E0E6EA");
    const QString active = dark ? QStringLiteral("#313B31") : QStringLiteral("#E7EFD0");

    return QStringLiteral(R"(
        QMainWindow, QWidget#root, QStackedWidget#pageStack { background: %1; }
        QWidget { color: %2; font-family: Inter, "Segoe UI", Arial, sans-serif; font-size: 14px; }
        QFrame#sidebar { background: %3; border-right: 1px solid %4; }
        QFrame#card { background: %5; border: 1px solid %4; border-radius: %6px; }
        QLabel#muted { color: %7; }
        QLabel#pageTitle { font-size: 28px; font-weight: 700; }
        QLabel#sectionTitle { font-size: 18px; font-weight: 700; }
        QLabel#environmentBadge { color: #A7C900; font-size: 10px; font-weight: 800; letter-spacing: 1px; padding: 5px 8px; }
        QLabel#statusNeutral { color: %7; }
        QLabel#statusSuccess { color: #A7C900; }
        QLabel#statusError { color: #E15A5A; }
        QLabel#statusWarning { color: #D49A2D; }
        QFrame#statusBar { background: %9; border: 1px solid %4; border-radius: 10px; }
        QLabel#address { background: #F2F0E8; color: #111315; border-radius: 14px; padding: 20px; font-family: Consolas, monospace; }
        QLabel#success { color: #A7C900; }
        QLabel#error { color: #E15A5A; }
        QPushButton { min-height: 20px; background: #D7FF4F; color: #101214; border: 0; border-radius: %8px; padding: 12px 17px; font-weight: 700; }
        QPushButton:hover { background: #E5FF86; }
        QPushButton:focus { border: 2px solid #A7C900; padding: 10px 15px; }
        QPushButton#danger { background: #C94747; color: #FFFFFF; }
        QPushButton#danger:hover { background: #D75A5A; }
        QPushButton:disabled { background: #B5BCC2; color: #69737C; }
        QPushButton#secondary, QPushButton#nav { background: %9; color: %2; border: 1px solid transparent; }
        QPushButton#nav { text-align: left; padding: 12px 14px; }
        QPushButton#nav:hover, QPushButton#secondary:hover { background: %10; border-color: %4; }
        QPushButton#nav[active="true"] { background: %12; color: %2; border: 1px solid #7E9400; }
        QLineEdit, QSpinBox, QComboBox, QTableWidget { background: %11; border: 1px solid %4; border-radius: 9px; padding: 8px; }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QTableWidget:focus { border: 1px solid #A7C900; }
        QHeaderView::section { background: %9; color: %2; padding: 8px; border: 0; }
        QTableWidget { gridline-color: %4; selection-background-color: %10; }
        QProgressBar { border: 1px solid %4; border-radius: 7px; background: %11; text-align: center; }
        QProgressBar::chunk { background: #D7FF4F; border-radius: 6px; }
        QScrollArea#navigationScroll { background: transparent; border: 0; }
        QScrollArea#navigationScroll > QWidget > QWidget { background: transparent; }
        QScrollBar:vertical { background: transparent; width: 6px; margin: 2px; }
        QScrollBar::handle:vertical { background: %4; border-radius: 3px; min-height: 24px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QToolTip { background: %5; color: %2; border: 1px solid %4; padding: 6px; }
    )")
        .arg(background, text, sidebar, border, surface)
        .arg(DesignTokens::CardRadius)
        .arg(muted)
        .arg(DesignTokens::ControlRadius)
        .arg(control, hover, input, active);
}
