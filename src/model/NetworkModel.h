#ifndef NETWORK_MODEL_H
#define NETWORK_MODEL_H

#include <QMetaType>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>

struct NetworkLink
{
    bool present = false;
    bool linkUp = false;
    QString ip;
    int speedMbps = -1;
    QString ssid;
    int signalDbm = 0;

    bool online() const { return linkUp && !ip.isEmpty(); }
    bool weakSignal() const { return signalDbm < -70; }

    bool operator==(const NetworkLink &other) const
    {
        return present == other.present && linkUp == other.linkUp && ip == other.ip
            && speedMbps == other.speedMbps && ssid == other.ssid && signalDbm == other.signalDbm;
    }
};

struct NetworkStatus
{
    NetworkLink ethernet;
    NetworkLink wifi;

    bool online() const { return ethernet.online() || wifi.online(); }

    bool operator==(const NetworkStatus &other) const
    {
        return ethernet == other.ethernet && wifi == other.wifi;
    }
};

Q_DECLARE_METATYPE(NetworkStatus)

class NetworkModel : public QObject
{
    Q_OBJECT

public:
    // rootPath trỏ tới cây thư mục giả khi test, mặc định là / của hệ thống.
    explicit NetworkModel(QObject *parent = nullptr, const QString &rootPath = QString(),
                          const QString &ethernetName = QString(), const QString &wifiName = QString());

    void start();

    NetworkStatus status() const { return mStatus; }

public slots:
    void refresh();
    void refreshSsid();

signals:
    void statusChanged(const NetworkStatus &status);

private:
    NetworkLink readLink(const QString &name) const;
    int readSignalDbm() const;

    QString mRoot;
    QString mEthernetName;
    QString mWifiName;
    QString mSsid;
    NetworkStatus mStatus;
    bool mHasStatus = false;
    QProcess mWpaCli;
    QTimer mPoll;
};

#endif // NETWORK_MODEL_H
