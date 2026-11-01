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
    mUptime = mList->addRow(QString());
    mKernel = mList->addRow(QString());
    mRam = mList->addRow(QString());
    mData = mList->addBarRow(QString());
    mMac = mList->addRow(QString());
    mIp = mList->addRow(QString());
    mLink = mList->addRow(QString());
    mMqtt = mList->addRow(QString());
    mList->place(8, 6);

    Language::bind(this, [this]() {
        setTitle(tr("Thiết bị & mạng"));
        mList->setLabel(mUptime, tr("Thời gian chạy"));
        mList->setLabel(mKernel, QStringLiteral("Kernel"));
        mList->setLabel(mRam, tr("RAM đã dùng"));
        mList->setLabel(mData, tr("Bộ nhớ /data"));
        mList->setLabel(mMac, QStringLiteral("MAC"));
        mList->setLabel(mIp, QStringLiteral("IP"));
        mList->setLabel(mLink, tr("Liên kết eth0"));
        mList->setLabel(mMqtt, QStringLiteral("MQTT"));
        if (mHasMqtt) {
            setMqttConnected(mMqttConnected);
        }
    });

    mTick->setInterval(kTickMs);
    connect(mTick, &QTimer::timeout, this, &DevicePage::refreshRequested);
}

void DevicePage::setDeviceInfo(const DeviceInfo &info)
{
    mList->setValue(mUptime, uptimeText(info.uptimeSec));
    mList->setValue(mKernel, info.kernel.isEmpty() ? QStringLiteral("--") : info.kernel);
    mList->setValue(mRam, info.ramTotalMb > 0 ? QStringLiteral("%1/%2 MB").arg(info.ramUsedMb).arg(info.ramTotalMb)
                                              : QStringLiteral("--"));
    mList->setValue(mData, info.dataPercent >= 0 ? QStringLiteral("%1%").arg(info.dataPercent) : QStringLiteral("--"));
    mList->setBar(mData, qMax(0, info.dataPercent));
    mList->setValue(mIp, info.ip.isEmpty() ? QStringLiteral("--") : info.ip);

    if (!info.linkUp) {
        mList->setValue(mLink, tr("Mất liên kết"), KeyValueList::Bad);
    } else if (info.linkMbps > 0) {
        mList->setValue(mLink, QStringLiteral("%1 Mb/s").arg(info.linkMbps), KeyValueList::Good);
    } else {
        mList->setValue(mLink, tr("Đã kết nối"), KeyValueList::Good);
    }
}

void DevicePage::setMacAddress(const QString &mac)
{
    mList->setValue(mMac, mac.isEmpty() ? QStringLiteral("--") : mac);
}

void DevicePage::setMqttConnected(bool connected)
{
    mHasMqtt = true;
    mMqttConnected = connected;
    if (connected) {
        mList->setValue(mMqtt, tr("Đã kết nối"), KeyValueList::Good);
    } else {
        mList->setValue(mMqtt, tr("Mất kết nối"), KeyValueList::Bad);
    }
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
