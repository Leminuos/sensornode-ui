#include "model/DeviceInfoModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QFile>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QSysInfo>

static QByteArray readFile(const QString &path)
{
    QFile file(path);
    return file.open(QFile::ReadOnly) ? file.readAll() : QByteArray();
}

static qint64 meminfoKb(const QByteArray &meminfo, const char *key)
{
    const QRegularExpression pattern(QStringLiteral("^%1:\\s+(\\d+) kB").arg(QLatin1String(key)),
                                     QRegularExpression::MultilineOption);
    const QRegularExpressionMatch match = pattern.match(QString::fromLatin1(meminfo));
    return match.hasMatch() ? match.captured(1).toLongLong() : -1;
}

static bool mountIsReadOnly(const QByteArray &mounts, const char *mountPoint)
{
    for (const QByteArray &line : mounts.split('\n')) {
        const QList<QByteArray> fields = line.split(' ');
        if (fields.size() >= 4 && fields.at(1) == mountPoint && fields.at(0) != "rootfs") {
            return fields.at(3).split(',').contains("ro");
        }
    }
    return false;
}

DeviceInfoModel::DeviceInfoModel(QObject *parent, const QString &rootPath, const QString &dataMount)
    : QObject(parent)
    , mRoot(rootPath)
    , mDataMount(dataMount.isEmpty() ? DaemonEndpoints::dataMount() : dataMount)
{
}

DeviceInfo DeviceInfoModel::read() const
{
    DeviceInfo info;

    const QByteArray uptime = readFile(mRoot + QStringLiteral("/proc/uptime"));
    if (!uptime.isEmpty()) {
        info.uptimeSec = static_cast<qint64>(uptime.split(' ').first().toDouble());
    }

    info.kernel = QSysInfo::kernelVersion();

    const QByteArray meminfo = readFile(mRoot + QStringLiteral("/proc/meminfo"));
    const qint64 totalKb = meminfoKb(meminfo, "MemTotal");
    const qint64 availableKb = meminfoKb(meminfo, "MemAvailable");
    if (totalKb > 0 && availableKb >= 0) {
        info.ramTotalMb = static_cast<int>(totalKb / 1024);
        info.ramUsedMb = static_cast<int>((totalKb - availableKb) / 1024);
    }

    const QStorageInfo storage(mDataMount);
    if (storage.isValid() && storage.bytesTotal() > 0) {
        const qint64 used = storage.bytesTotal() - storage.bytesAvailable();
        info.dataPercent = static_cast<int>(used * 100 / storage.bytesTotal());
    }

    info.rootReadOnly = mountIsReadOnly(readFile(mRoot + QStringLiteral("/proc/mounts")), "/");

    const QByteArray cmdline = readFile(mRoot + QStringLiteral("/proc/cmdline"));
    info.verityEnabled = cmdline.contains("root=/dev/dm-") || cmdline.contains("root=/dev/mapper/");

    return info;
}

void DeviceInfoModel::refresh()
{
    emit infoChanged(read());
}
