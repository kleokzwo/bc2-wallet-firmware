#include "appsettings.h"

#include <QSettings>

namespace {
constexpr const char *kElectrumHostKey = "network/electrumHost";
constexpr const char *kElectrumPortKey = "network/electrumPort";
constexpr const char *kElectrumSslKey = "network/electrumSsl";
constexpr const char *kLightThemeKey = "appearance/lightTheme";
constexpr const char *kWindowGeometryKey = "window/geometry";
constexpr const char *kReceiveGapLimitKey = "receive/gapLimit";
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


int AppSettings::receiveGapLimit() const {
    return settings_->value(kReceiveGapLimitKey, 20).toInt();
}

QString AppSettings::receiveLabel(unsigned int index) const {
    return settings_->value(QStringLiteral("receive/labels/%1").arg(index)).toString();
}

void AppSettings::setReceiveGapLimit(int gapLimit) {
    settings_->setValue(kReceiveGapLimitKey, gapLimit);
}

void AppSettings::setReceiveLabel(unsigned int index, const QString &label) {
    const QString key = QStringLiteral("receive/labels/%1").arg(index);
    if (label.trimmed().isEmpty()) settings_->remove(key);
    else settings_->setValue(key, label.trimmed());
}
