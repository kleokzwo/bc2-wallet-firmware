#ifndef BC2_SIMULATOR_APPSETTINGS_H
#define BC2_SIMULATOR_APPSETTINGS_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>

#include <memory>

class QSettings;

class AppSettings final {
public:
    AppSettings();
    ~AppSettings();

    QString electrumHost() const;
    quint16 electrumPort() const;
    bool electrumSslEnabled() const;
    bool lightThemeEnabled() const;
    QByteArray windowGeometry() const;

    void setElectrumServer(const QString &host, quint16 port, bool sslEnabled);
    void setLightThemeEnabled(bool enabled);
    void setWindowGeometry(const QByteArray &geometry);

private:
    std::unique_ptr<QSettings> settings_;
};

#endif
