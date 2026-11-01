#include "screens/DevicePage.h"

#include "app/Language.h"
#include "widgets/KeyValueList.h"

#include <QTimer>

static constexpr int kTickMs = 1000;

QString DevicePage::uptimeText(qint64 seconds)
{
    if (seconds < 0) return QStringLiteral("--");

    const qint64 days = seconds / 86400;
    const QString clock = QStringLiteral("%1:%2")
                              .arg((seconds % 86400) / 3600, 2, 10, QLatin1Char('0'))
                              .arg((seconds % 3600) / 60, 2, 10, QLatin1Char('0'));
    return days > 0 ? tr("%1 ngày %2").arg(days).arg(clock) : clock;
}

DevicePage::DevicePage(QWidget *parent)
    : SubPage(QString(), parent)
    , mList(new KeyValueList(content()))
    , mTick(new QTimer(this))
{
    mNodeId = mList->addRow(QStringLiteral("Node ID"));
    mMac = mList->addRow(QStringLiteral("MAC"));
    mUptime = mList->addRow(QString());
    mList->place(8, 6);

    Language::bind(this, [this]() {
        setTitle(tr("Thiết bị"));
        mList->setLabel(mUptime, tr("Thời gian chạy"));
    });

    mTick->setInterval(kTickMs);
    connect(mTick, &QTimer::timeout, this, &DevicePage::refreshRequested);
}

void DevicePage::setDeviceInfo(const DeviceInfo &info)
{
    mList->setValue(mUptime, uptimeText(info.uptimeSec));
}

void DevicePage::setNodeId(const QString &nodeId)
{
    mList->setValue(mNodeId, nodeId.isEmpty() ? QStringLiteral("--") : nodeId);
}

void DevicePage::setMacAddress(const QString &mac)
{
    mList->setValue(mMac, mac.isEmpty() ? QStringLiteral("--") : mac);
}

void DevicePage::showEvent(QShowEvent *event)
{
    SubPage::showEvent(event);
    emit refreshRequested();
    mTick->start();
}

void DevicePage::hideEvent(QHideEvent *event)
{
    SubPage::hideEvent(event);
    mTick->stop();
}
