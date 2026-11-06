#include "model/PowerModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QDebug>

PowerModel::PowerModel(QObject *parent, const QString &socketPath)
    : QObject(parent)
    , mClient(socketPath.isEmpty() ? DaemonEndpoints::powerSocket() : socketPath)
{
    connect(&mClient, &DaemonClient::messageReceived, this, &PowerModel::onMessage);
}

void PowerModel::start()
{
    mClient.start();
}

void PowerModel::reboot()
{
    if (mRebootId >= 0) return;

    mRebootId = mClient.sendCommand(QStringLiteral("reboot"));
    if (mRebootId < 0) {
        qWarning() << "powerd: not connected";
        emit rebootFailed();
    }
}

void PowerModel::onMessage(const QJsonObject &message)
{
    if (message.value(QStringLiteral("type")).toString() != QLatin1String("reply")
        || message.value(QStringLiteral("id")).toInt(-2) != mRebootId) {
        return;
    }

    mRebootId = -1;
    if (!message.value(QStringLiteral("ok")).toBool()) {
        qWarning() << "powerd rejected reboot:" << message.value(QStringLiteral("error")).toString();
        emit rebootFailed();
    }
}
