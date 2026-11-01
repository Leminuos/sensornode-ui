#include "model/SettingsModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QDebug>

static const QString kBrightness = QStringLiteral("brightness");
static const QString kAutoMode = QStringLiteral("auto_mode");

SettingsModel::SettingsModel(QObject *parent, const QString &socketPath)
    : QObject(parent)
    , mClient(socketPath.isEmpty() ? DaemonEndpoints::settingsSocket() : socketPath)
{
    connect(&mClient, &DaemonClient::disconnected, this, [this]() {
        emit availableChanged(false);
    });
    connect(&mClient, &DaemonClient::messageReceived, this, &SettingsModel::onMessage);
}

void SettingsModel::start()
{
    mClient.start();
}

void SettingsModel::setBrightness(int value)
{
    set(kBrightness, value);
}

void SettingsModel::setAutoMode(bool autoMode)
{
    if (autoMode == mAutoMode) return;

    mAutoMode = autoMode;
    set(kAutoMode, autoMode);
    emit autoModeChanged(autoMode);
}

void SettingsModel::set(const QString &key, const QJsonValue &value)
{
    mClient.sendCommand(QStringLiteral("set"), {{QStringLiteral("key"), key}, {QStringLiteral("value"), value}});
}

void SettingsModel::onMessage(const QJsonObject &message)
{
    const QString type = message.value(QStringLiteral("type")).toString();

    if (type == QLatin1String("hello")) {
        const QJsonObject settings = message.value(QStringLiteral("settings")).toObject();
        emit brightnessChanged(settings.value(kBrightness).toInt());
        mAutoMode = settings.value(kAutoMode).toBool();
        emit autoModeChanged(mAutoMode);
        emit availableChanged(true);
    } else if (type == QLatin1String("reply") && !message.value(QStringLiteral("ok")).toBool()) {
        qWarning() << "settingsd rejected" << message.value(QStringLiteral("error")).toString();
    }
}
