#include "model/NetworkModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QAbstractSocket>
#include <QFile>
#include <QFileInfo>
#include <QNetworkInterface>

static constexpr int kPollMs = 2000;

static QByteArray readFile(const QString &path)
{
    QFile file(path);
    return file.open(QFile::ReadOnly) ? file.readAll() : QByteArray();
}

static QString ipv4Address(const QString &name)
{
    const auto entries = QNetworkInterface::interfaceFromName(name).addressEntries();
    for (const QNetworkAddressEntry &entry : entries) {
        if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
            return entry.ip().toString();
        }
    }
    return QString();
}

NetworkModel::NetworkModel(QObject *parent, const QString &rootPath, const QString &ethernetName,
                           const QString &wifiName)
    : QObject(parent)
    , mRoot(rootPath)
    , mEthernetName(ethernetName.isEmpty() ? DaemonEndpoints::networkInterface() : ethernetName)
    , mWifiName(wifiName.isEmpty() ? DaemonEndpoints::wifiInterface() : wifiName)
{
    mPoll.setInterval(kPollMs);
    connect(&mPoll, &QTimer::timeout, this, &NetworkModel::refresh);

    connect(&mWpaCli, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this]() {
        const QList<QByteArray> lines = mWpaCli.readAllStandardOutput().split('\n');
        for (const QByteArray &line : lines) {
            if (line.startsWith("ssid=")) {
                mSsid = QString::fromUtf8(line.mid(5));
            }
        }
        refresh();
    });
}

void NetworkModel::start()
{
    refresh();
    mPoll.start();
}

NetworkLink NetworkModel::readLink(const QString &name) const
{
    NetworkLink link;
    const QString sysNet = mRoot + QStringLiteral("/sys/class/net/") + name;
    link.present = QFileInfo::exists(sysNet);
    if (!link.present) return link;

    link.linkUp = readFile(sysNet + QStringLiteral("/carrier")).trimmed() == "1";
    if (!link.linkUp) return link;

    link.ip = ipv4Address(name);
    bool speedOk = false;
    const int speed = readFile(sysNet + QStringLiteral("/speed")).trimmed().toInt(&speedOk);
    if (speedOk && speed > 0) {
        link.speedMbps = speed;
    }
    return link;
}

int NetworkModel::readSignalDbm() const
{
    const QList<QByteArray> lines = readFile(mRoot + QStringLiteral("/proc/net/wireless")).split('\n');
    for (const QByteArray &line : lines) {
        const QList<QByteArray> fields = line.simplified().split(' ');
        if (fields.size() >= 4 && fields.first() == mWifiName.toLatin1() + ':') {
            return static_cast<int>(fields.at(3).toDouble());
        }
    }
    return 0;
}

void NetworkModel::refresh()
{
    const bool wifiWasUp = mStatus.wifi.linkUp;

    NetworkStatus status;
    status.ethernet = readLink(mEthernetName);
    status.wifi = readLink(mWifiName);
    status.wifi.speedMbps = -1;
    if (status.wifi.linkUp) {
        status.wifi.ssid = mSsid;
        status.wifi.signalDbm = readSignalDbm();
    } else {
        mSsid.clear();
    }

    if (!mHasStatus || !(status == mStatus)) {
        mHasStatus = true;
        mStatus = status;
        emit statusChanged(mStatus);
    }

    if (status.wifi.linkUp && !wifiWasUp) {
        refreshSsid();
    }
}

void NetworkModel::refreshSsid()
{
    if (!mStatus.wifi.linkUp || mWpaCli.state() != QProcess::NotRunning) return;

    mWpaCli.start(DaemonEndpoints::wpaCli(), {QStringLiteral("-i"), mWifiName, QStringLiteral("status")});
}
