#include "model/SystemStatusModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QFileInfo>
#include <QTime>

static constexpr int kPollMs = 2000;

SystemStatusModel::SystemStatusModel(QObject *parent, const QString &timeSyncFlag)
    : QObject(parent)
    , mTimeSyncFlag(timeSyncFlag.isEmpty() ? DaemonEndpoints::timeSyncFlag() : timeSyncFlag)
{
    mPoll.setInterval(kPollMs);
    connect(&mPoll, &QTimer::timeout, this, &SystemStatusModel::refresh);
}

void SystemStatusModel::start()
{
    refresh();
    mPoll.start();
}

void SystemStatusModel::refresh()
{
    const QString clock = QFileInfo::exists(mTimeSyncFlag)
        ? QTime::currentTime().toString(QStringLiteral("HH:mm"))
        : QString();
    if (clock != mClockText) {
        mClockText = clock;
        emit clockChanged(mClockText);
    }
}
