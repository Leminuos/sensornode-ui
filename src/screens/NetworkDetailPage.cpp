#include "screens/NetworkDetailPage.h"

#include "app/Language.h"
#include "widgets/KeyValueList.h"

static const char *const kDash = "--";

NetworkDetailPage::NetworkDetailPage(Kind kind, QWidget *parent)
    : SubPage(kind == Ethernet ? QStringLiteral("Ethernet") : QStringLiteral("Wi-Fi"), parent)
    , mKind(kind)
    , mList(new KeyValueList(content()))
{
    mFirst = mList->addRow(QString());
    mSecond = mList->addRow(QString());
    mIp = mList->addRow(QString());
    mList->place(8, 6);

    Language::bind(this, [this]() {
        mList->setLabel(mFirst, mKind == Ethernet ? tr("Liên kết") : tr("Tên mạng"));
        mList->setLabel(mSecond, mKind == Ethernet ? tr("Tốc độ") : tr("Tín hiệu"));
        mList->setLabel(mIp, tr("Địa chỉ IP"));
        refreshTexts();
    });
}

void NetworkDetailPage::setStatus(const NetworkStatus &status)
{
    mLink = mKind == Ethernet ? status.ethernet : status.wifi;
    refreshTexts();
}

void NetworkDetailPage::refreshTexts()
{
    const QString dash = QLatin1String(kDash);

    if (mKind == Ethernet) {
        if (mLink.linkUp) {
            mList->setValue(mFirst, tr("Đã nối"), KeyValueList::Good);
        } else {
            mList->setValue(mFirst, dash);
        }
        mList->setValue(mSecond, mLink.speedMbps > 0 ? QStringLiteral("%1 Mb/s").arg(mLink.speedMbps) : dash);
    } else {
        if (mLink.ssid.isEmpty()) {
            mList->setValue(mFirst, dash);
        } else {
            mList->setElidedValue(mFirst, mLink.ssid, KeyValueList::Good);
        }
        mList->setValue(mSecond, mLink.signalDbm != 0 ? QStringLiteral("%1 dBm").arg(mLink.signalDbm) : dash);
    }

    mList->setValue(mIp, mLink.ip.isEmpty() ? dash : mLink.ip);
}

void NetworkDetailPage::showEvent(QShowEvent *event)
{
    SubPage::showEvent(event);
    emit refreshRequested();
}
