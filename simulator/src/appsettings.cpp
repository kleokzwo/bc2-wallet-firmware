#include "appsettings.h"

#include <QSettings>

namespace {
constexpr const char *kElectrumHostKey = "network/electrumHost";
constexpr const char *kElectrumPortKey = "network/electrumPort";
constexpr const char *kElectrumSslKey = "network/electrumSsl";
constexpr const char *kLightThemeKey = "appearance/lightTheme";
constexpr const char *kWindowGeometryKey = "window/geometry";
constexpr const char *kDefaultElectrumHost = "infra1.bitcoin-ii.org";
constexpr int kDefaultElectrumPort = 50009;
}

AppSettings::AppSettings() : settings_(std::make_unique<QSettings>()) {}

AppSettings::~AppSettings() = default;

QString AppSettings::electrumHost() const {
    return settings_->value(kElectrumHostKey, QString::fromLatin1(kDefaultElectrumHost)).toString();
}

quint16 AppSettings::electrumPort() const {
    return static_cast<quint16>(settings_->value(kElectrumPortKey, kDefaultElectrumPort).toUInt());
}

bool AppSettings::electrumSslEnabled() const {
    return settings_->value(kElectrumSslKey, true).toBool();
}

bool AppSettings::lightThemeEnabled() const {
    return settings_->value(kLightThemeKey, false).toBool();
}

QByteArray AppSettings::windowGeometry() const {
    return settings_->value(kWindowGeometryKey).toByteArray();
}

void AppSettings::setElectrumServer(const QString &host, quint16 port, bool sslEnabled) {
    settings_->setValue(kElectrumHostKey, host.trimmed());
    settings_->setValue(kElectrumPortKey, port);
    settings_->setValue(kElectrumSslKey, sslEnabled);
}

void AppSettings::setLightThemeEnabled(bool enabled) {
    settings_->setValue(kLightThemeKey, enabled);
}

void AppSettings::setWindowGeometry(const QByteArray &geometry) {
    settings_->setValue(kWindowGeometryKey, geometry);
}
