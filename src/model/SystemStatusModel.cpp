#include "model/SystemStatusModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QAbstractSocket>
#include <QFileInfo>
#include <QNetworkInterface>
#include <QTime>

static constexpr int kPollMs = 2000;

static bool interfaceOnline(const QString &name)
{
    const QNetworkInterface iface = QNetworkInterface::interfaceFromName(name);
    if (!iface.isValid() || !iface.flags().testFlag(QNetworkInterface::IsRunning)) {
        return false;
    }

    const auto entries = iface.addressEntries();
    for (const QNetworkAddressEntry &entry : entries) {
        if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
            return true;
        }
    }
    return false;
}

SystemStatusModel::SystemStatusModel(QObject *parent, const QString &timeSyncFlag, const QString &interfaceName)
    : QObject(parent)
    , mTimeSyncFlag(timeSyncFlag.isEmpty() ? DaemonEndpoints::timeSyncFlag() : timeSyncFlag)
    , mInterfaceName(interfaceName.isEmpty() ? DaemonEndpoints::networkInterface() : interfaceName)
{
    mPoll.setInterval(kPollMs);
    connect(&mPoll, &QTimer::timeout, this, &SystemStatusModel::poll);
}

void SystemStatusModel::start()
{
    poll();
    mPoll.start();
}

void SystemStatusModel::poll()
{
    const QString clock = QFileInfo::exists(mTimeSyncFlag)
        ? QTime::currentTime().toString(QStringLiteral("HH:mm"))
        : QString();
    if (clock != mClockText) {
        mClockText = clock;
        emit clockChanged(mClockText);
    }

    const bool up = interfaceOnline(mInterfaceName);
    if (up != mNetworkUp) {
        mNetworkUp = up;
        emit networkChanged(mNetworkUp);
    }
}
