#ifndef SYSTEM_PAGE_H
#define SYSTEM_PAGE_H

#include "model/DeviceInfoModel.h"
#include "widgets/SubPage.h"

class KeyValueList;
class QTimer;

class SystemPage : public SubPage
{
    Q_OBJECT

public:
    explicit SystemPage(QWidget *parent = nullptr);

public slots:
    void setDeviceInfo(const DeviceInfo &info);
    void setFirmwareVersion(const QString &version);

signals:
    void refreshRequested();
    void restartRequested();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    KeyValueList *mList;
    QTimer *mTick;
    int mVersion;
    int mKernel;
    int mRam;
    int mData;
};

#endif // SYSTEM_PAGE_H
