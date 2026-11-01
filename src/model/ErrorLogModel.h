#ifndef ERROR_LOG_MODEL_H
#define ERROR_LOG_MODEL_H

#include <QDateTime>
#include <QMetaType>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>
#include <QVector>

struct LogEntry
{
    QDateTime time;
    QString source;
    QString message;
};

Q_DECLARE_METATYPE(LogEntry)

class ErrorLogModel : public QObject
{
    Q_OBJECT

public:
    explicit ErrorLogModel(QObject *parent = nullptr, const QString &program = QString());

    static QVector<LogEntry> parse(const QByteArray &journalJson);

public slots:
    void reload();

signals:
    void entriesChanged(const QVector<LogEntry> &entries);
    void failed();

private:
    void onFinished(int exitCode, QProcess::ExitStatus status);

    QString mProgram;
    QProcess mProcess;
    QTimer mTimeout;
};

#endif // ERROR_LOG_MODEL_H
