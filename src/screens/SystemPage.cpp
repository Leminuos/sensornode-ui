#include "screens/SystemPage.h"

#include "app/Language.h"
#include "widgets/KeyValueList.h"
#include "widgets/RaisedButton.h"

#include <QTimer>

static constexpr int kTickMs = 1000;

SystemPage::SystemPage(QWidget *parent)
    : SubPage(QString(), parent)
    , mList(new KeyValueList(content()))
    , mTick(new QTimer(this))
{
    mVersion = mList->addRow(QString());
    mKernel = mList->addRow(QStringLiteral("Kernel"));
    mRam = mList->addRow(QString());
    mData = mList->addBarRow(QString());
    mList->place(8, 6);

    auto *restart = new RaisedButton(QString(), RaisedButton::Secondary, content());
    restart->setVisualGeometry(QRect(8, 208, 224, 42));
    connect(restart, &QPushButton::clicked, this, &SystemPage::restartRequested);

    Language::bind(this, [=]() {
        setTitle(tr("Hệ thống"));
        mList->setLabel(mVersion, tr("Phiên bản firmware"));
        mList->setLabel(mRam, tr("RAM đã dùng"));
        mList->setLabel(mData, tr("Bộ nhớ /data"));
        restart->setText(tr("Khởi động lại"));
    });

    mTick->setInterval(kTickMs);
    connect(mTick, &QTimer::timeout, this, &SystemPage::refreshRequested);
}

void SystemPage::setDeviceInfo(const DeviceInfo &info)
{
    mList->setValue(mKernel, info.kernel.isEmpty() ? QStringLiteral("--") : info.kernel);
    mList->setValue(mRam, info.ramTotalMb > 0 ? QStringLiteral("%1/%2 MB").arg(info.ramUsedMb).arg(info.ramTotalMb)
                                              : QStringLiteral("--"));
    mList->setValue(mData, info.dataPercent >= 0 ? QStringLiteral("%1%").arg(info.dataPercent) : QStringLiteral("--"));
    mList->setBar(mData, qMax(0, info.dataPercent));
}

void SystemPage::setFirmwareVersion(const QString &version)
{
    mList->setValue(mVersion, version.isEmpty() ? QStringLiteral("--") : version);
}

void SystemPage::showEvent(QShowEvent *event)
{
    SubPage::showEvent(event);
    emit refreshRequested();
    mTick->start();
}

void SystemPage::hideEvent(QHideEvent *event)
{
    SubPage::hideEvent(event);
    mTick->stop();
}
