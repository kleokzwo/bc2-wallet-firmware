#ifndef BC2_SIMULATOR_THEMEMANAGER_H
#define BC2_SIMULATOR_THEMEMANAGER_H

#include <QObject>
#include <QString>

class QApplication;

class ThemeManager final : public QObject {
    Q_OBJECT
public:
    enum class Theme {
        Dark,
        Light
    };
    Q_ENUM(Theme)

    explicit ThemeManager(QApplication *application, QObject *parent = nullptr);

    Theme theme() const;
    QString themeName() const;
    void setTheme(Theme theme);
    void toggleTheme();

signals:
    void themeChanged(Theme theme);

private:
    QString styleSheetFor(Theme theme) const;

    QApplication *application_;
    Theme theme_ = Theme::Dark;
};

#endif
