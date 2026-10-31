#include "settings/OtaSettings.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

OtaSettings::OtaSettings(const QString &path)
    : mPath(path.isEmpty() ? qEnvironmentVariable("OTA_CONFIG_FILE", QStringLiteral("/data/config/ota.json")) : path)
{
}

void OtaSettings::load()
{
    QFile f(mPath);
    if (!f.open(QIODevice::ReadOnly)) return;

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "OtaSettings: corrupt config file" << mPath << err.errorString();
        return;
    }

    mRoot = doc.object();
}

bool OtaSettings::save() const
{
    const QString dir = QFileInfo(mPath).absolutePath();
    if (!QDir().mkpath(dir)) {
        qWarning() << "OtaSettings: cannot create dir" << dir;
        return false;
    }

    QFile f(mPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "OtaSettings: cannot write" << mPath << f.errorString();
        return false;
    }
    
    f.write(QJsonDocument(mRoot).toJson(QJsonDocument::Indented));
    return true;
}

bool OtaSettings::autoMode() const
{
    return mRoot.value(QStringLiteral("auto-mode")).toBool(false);
}

void OtaSettings::setAutoMode(bool autoMode)
{
    mRoot.insert(QStringLiteral("auto-mode"), autoMode);
}
