#ifndef FIRMWARE_PAGE_H
#define FIRMWARE_PAGE_H

#include "model/DeviceInfoModel.h"
#include "model/OtaState.h"
#include "widgets/SubPage.h"

class KeyValueList;

class FirmwarePage : public SubPage
{
    Q_OBJECT

public:
    explicit FirmwarePage(QWidget *parent = nullptr);

public slots:
    void setSystemInfo(const OtaSystemInfo &info);
    void setDeviceInfo(const DeviceInfo &info);

signals:
    void refreshRequested();
    void restartRequested();

protected:
    void showEvent(QShowEvent *event) override;

private:
    KeyValueList *mList;
    int mVersion;
    int mRunningSlot;
    int mNextSlot;
    int mBootState;
    int mAttempts;
    int mRootfs;
    int mVerity;
};

#endif // FIRMWARE_PAGE_H
