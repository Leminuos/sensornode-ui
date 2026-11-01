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
    void setNodeId(const QString &nodeId);
    void setMacAddress(const QString &mac);

signals:
    void refreshRequested();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    KeyValueList *mList;
    QTimer *mTick;
    int mNodeId;
    int mMac;
    int mUptime;
};

#endif // DEVICE_PAGE_H
