#ifndef DEVICE_PAGE_H
#define DEVICE_PAGE_H

#include "model/DeviceInfoModel.h"
#include "widgets/SubPage.h"

class KeyValueList;
class QTimer;

class DevicePage : public SubPage
{
    Q_OBJECT

public:
    explicit DevicePage(QWidget *parent = nullptr);

    static QString uptimeText(qint64 seconds);

public slots:
    void setDeviceInfo(const DeviceInfo &info);
    void setMacAddress(const QString &mac);
    void setMqttConnected(bool connected);

signals:
    void refreshRequested();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    KeyValueList *mList;
    QTimer *mTick;
    int mUptime;
    int mKernel;
    int mRam;
    int mData;
    int mMac;
    int mIp;
    int mLink;
    int mMqtt;
};

#endif // DEVICE_PAGE_H
