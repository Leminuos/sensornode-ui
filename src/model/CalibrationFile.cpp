#include "model/CalibrationFile.h"

#include "ipc/DaemonEndpoints.h"

#include <QDebug>
#include <QFile>
#include <QSaveFile>

static QString backupPath(const QString &path)
{
    return path + QStringLiteral(".bak");
}

static QString pendingPath(const QString &path)
{
    return path + QStringLiteral(".pending");
}

static bool writeFile(const QString &path, const QByteArray &data)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size() || !file.commit()) {
        qWarning() << "Calibration: cannot write" << path << file.errorString();
        return false;
    }
    return true;
}

static QByteArray readFile(const QString &path)
{
    QFile file(path);
    return file.open(QFile::ReadOnly) ? file.readAll() : QByteArray();
}

CalibrationFile::CalibrationFile(const QString &path)
    : mPath(path.isEmpty() ? DaemonEndpoints::touchCalibrationFile() : path)
{
}

bool CalibrationFile::isPending() const
{
    return QFile::exists(pendingPath(mPath));
}

bool CalibrationFile::apply(const QString &pointercal)
{
    if (!isPending() && !writeFile(backupPath(mPath), readFile(mPath))) return false;
    if (!writeFile(pendingPath(mPath), QByteArray())) return false;
    return writeFile(mPath, pointercal.toLatin1() + '\n');
}

void CalibrationFile::keep()
{
    QFile::remove(backupPath(mPath));
    QFile::remove(pendingPath(mPath));
}

bool CalibrationFile::revert()
{
    const QByteArray old = readFile(backupPath(mPath));
    const bool ok = old.isEmpty() ? (!QFile::exists(mPath) || QFile::remove(mPath)) : writeFile(mPath, old);
    if (ok) keep();
    return ok;
}
