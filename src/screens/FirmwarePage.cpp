#include "screens/FirmwarePage.h"

#include "widgets/KeyValueList.h"
#include "widgets/RaisedButton.h"

FirmwarePage::FirmwarePage(QWidget *parent)
    : SubPage(tr("Firmware"), parent)
    , mList(new KeyValueList(content()))
{
    mVersion = mList->addRow(tr("Phiên bản"));
    mRunningSlot = mList->addRow(tr("Slot đang chạy"));
    mNextSlot = mList->addRow(tr("Slot lần boot tới"));
    mBootState = mList->addRow(tr("Trạng thái boot"));
    mAttempts = mList->addRow(tr("Số lần thử"));
    mRootfs = mList->addRow(tr("Rootfs"));
    mVerity = mList->addChipRow(tr("dm-verity"));
    mList->place(8, 6);
    mList->setChip(mVerity, tr("Chưa bật"), "neutral");

    auto *restart = new RaisedButton(tr("Khởi động lại"), RaisedButton::Secondary, content());
    restart->setVisualGeometry(QRect(8, 208, 224, 42));
    connect(restart, &QPushButton::clicked, this, &FirmwarePage::restartRequested);
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
