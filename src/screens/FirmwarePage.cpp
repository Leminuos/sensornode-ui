#include "screens/FirmwarePage.h"

#include "app/Language.h"
#include "widgets/KeyValueList.h"

FirmwarePage::FirmwarePage(QWidget *parent)
    : SubPage(QString(), parent)
    , mList(new KeyValueList(content()))
{
    mVersion = mList->addRow(QString());
    mRunningSlot = mList->addRow(QString());
    mNextSlot = mList->addRow(QString());
    mBootState = mList->addRow(QString());
    mAttempts = mList->addRow(QString());
    mRootfs = mList->addRow(QString());
    mVerity = mList->addChipRow(QStringLiteral("dm-verity"));
    mList->place(8, 6);

    Language::bind(this, [=]() {
        setTitle(tr("Firmware"));
        mList->setLabel(mVersion, tr("Phiên bản"));
        mList->setLabel(mRunningSlot, tr("Slot đang chạy"));
        mList->setLabel(mNextSlot, tr("Slot lần boot tới"));
        mList->setLabel(mBootState, tr("Trạng thái boot"));
        mList->setLabel(mAttempts, tr("Số lần thử"));
        mList->setLabel(mRootfs, tr("Rootfs"));
        mList->setChip(mVerity, tr("Chưa bật"), "neutral");
    });
}

static QString orDash(const QString &text)
{
    return text.isEmpty() ? QStringLiteral("--") : text;
}

void FirmwarePage::setSystemInfo(const OtaSystemInfo &info)
{
    mList->setValue(mVersion, orDash(info.version));
    mList->setValue(mRunningSlot, orDash(info.runningSlot));
    mList->setValue(mNextSlot, orDash(info.activeSlot));

    if (info.trialBoot) {
        mList->setValue(mBootState, tr("Đang thử"));
    } else {
        mList->setValue(mBootState, tr("Đã xác nhận"), KeyValueList::Good);
    }

    const bool hasCount = info.bootCount >= 0 && info.bootLimit >= 0;
    mList->setValue(mAttempts, hasCount ? QStringLiteral("%1/%2").arg(info.bootCount).arg(info.bootLimit)
                                        : QStringLiteral("--"));
}

void FirmwarePage::setDeviceInfo(const DeviceInfo &info)
{
    mList->setValue(mRootfs, info.rootReadOnly ? tr("Chỉ đọc") : tr("Đọc ghi"));
    if (info.verityEnabled) {
        mList->setChip(mVerity, tr("Đã bật"), "ok");
    } else {
        mList->setChip(mVerity, tr("Chưa bật"), "neutral");
    }
}

void FirmwarePage::showEvent(QShowEvent *event)
{
    SubPage::showEvent(event);
    emit refreshRequested();
}
