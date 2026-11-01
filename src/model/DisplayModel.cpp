#include "model/DisplayModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QDebug>

DisplayModel::DisplayModel(QObject *parent, const QString &socketPath)
    : QObject(parent)
    , mClient(socketPath.isEmpty() ? DaemonEndpoints::displaySocket() : socketPath)
{
    connect(&mClient, &DaemonClient::messageReceived, this, &DisplayModel::onMessage);
}

void DisplayModel::start()
{
    mClient.start();
}

void DisplayModel::setBrightness(int value)
{
    mClient.sendCommand(QStringLiteral("set_brightness"), {{QStringLiteral("value"), value}});
}

void DisplayModel::onMessage(const QJsonObject &message)
{
    if (message.value(QStringLiteral("type")).toString() == QLatin1String("reply")
        && !message.value(QStringLiteral("ok")).toBool()) {
        qWarning() << "displayd rejected" << message.value(QStringLiteral("error")).toString();
    }
}
