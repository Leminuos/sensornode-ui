#include "model/ErrorLogModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringList>

static constexpr int kMaxEntries = 40;
static constexpr int kTimeoutMs = 5000;

ErrorLogModel::ErrorLogModel(QObject *parent, const QString &program)
    : QObject(parent)
    , mProgram(program.isEmpty() ? DaemonEndpoints::journalctl() : program)
{
    mTimeout.setSingleShot(true);
    mTimeout.setInterval(kTimeoutMs);
    connect(&mTimeout, &QTimer::timeout, &mProcess, &QProcess::kill);

    connect(&mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ErrorLogModel::onFinished);
    connect(&mProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            mTimeout.stop();
            emit failed();
        }
    });
}

void ErrorLogModel::reload()
{
    if (mProcess.state() != QProcess::NotRunning) return;

    QStringList args = {QStringLiteral("--no-pager"), QStringLiteral("--quiet"), QStringLiteral("--reverse"),
                        QStringLiteral("--output=json"), QStringLiteral("--priority=warning"),
                        QStringLiteral("--lines=%1").arg(kMaxEntries)};
    for (const char *daemon : {"telemetryd", "otad", "settingsd"}) {
        args << QStringLiteral("--unit=sensornode-%1.service").arg(QLatin1String(daemon));
    }

    mTimeout.start();
    mProcess.start(mProgram, args, QIODevice::ReadOnly);
}

void ErrorLogModel::onFinished(int exitCode, QProcess::ExitStatus status)
{
    mTimeout.stop();

    if (status != QProcess::NormalExit || exitCode != 0) {
        emit failed();
        return;
    }
    emit entriesChanged(parse(mProcess.readAllStandardOutput()));
}

QVector<LogEntry> ErrorLogModel::parse(const QByteArray &journalJson)
{
    static const QRegularExpression sourceTag(QStringLiteral("\\s+src=\\S+$"));

    QVector<LogEntry> entries;
    for (const QByteArray &line : journalJson.split('\n')) {
        const QJsonObject record = QJsonDocument::fromJson(line).object();

        // journald ghi MESSAGE dạng mảng byte nếu không phải UTF-8, các dòng đó bị bỏ qua.
        const QJsonValue message = record.value(QStringLiteral("MESSAGE"));
        if (!message.isString()) continue;

        LogEntry entry;
        entry.time = QDateTime::fromMSecsSinceEpoch(
            record.value(QStringLiteral("__REALTIME_TIMESTAMP")).toString().toLongLong() / 1000);
        entry.source = record.value(QStringLiteral("_SYSTEMD_UNIT")).toString()
                           .remove(QStringLiteral("sensornode-")).remove(QStringLiteral(".service"));
        entry.message = message.toString().remove(sourceTag);
        entries.append(entry);
    }
    return entries;
}
