#include "model/TelemetryModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QtNumeric>

static double reading(const QJsonObject &sample, const char *key)
{
    const QJsonValue v = sample.value(QLatin1String(key));
    return v.isDouble() ? v.toDouble() : qQNaN();
}

TelemetryModel::TelemetryModel(QObject *parent, const QString &socketPath)
    : QObject(parent)
    , mClient(socketPath.isEmpty() ? DaemonEndpoints::telemetrySocket() : socketPath)
{
    connect(&mClient, &DaemonClient::connected, this, [this]() {
        emit onlineChanged(true);
        mClient.sendCommand(QStringLiteral("get_latest"));
    });
    connect(&mClient, &DaemonClient::disconnected, this, [this]() {
        emit onlineChanged(false);
    });
    connect(&mClient, &DaemonClient::messageReceived, this, &TelemetryModel::onMessage);
}

void TelemetryModel::start()
{
    mClient.start();
}

bool TelemetryModel::isOnline() const
{
    return mClient.isConnected();
}

void TelemetryModel::onMessage(const QJsonObject &message)
{
    const QString type = message.value(QStringLiteral("type")).toString();

    if (type == QLatin1String("sample")) {
        emitSample(message);
    } else if (type == QLatin1String("reply")
               && message.value(QStringLiteral("cmd")).toString() == QLatin1String("get_latest")
               && message.value(QStringLiteral("ok")).toBool()) {
        emitSample(message.value(QStringLiteral("sample")).toObject());
    }
}

void TelemetryModel::emitSample(const QJsonObject &sample)
{
    emit sampleReceived(reading(sample, "temp_c"), reading(sample, "humi_rh"), reading(sample, "lux"));
}
