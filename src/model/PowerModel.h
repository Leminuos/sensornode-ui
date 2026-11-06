#ifndef POWER_MODEL_H
#define POWER_MODEL_H

#include <QJsonObject>
#include <QObject>
#include <QString>

#include "ipc/DaemonClient.h"

class PowerModel : public QObject
{
    Q_OBJECT

public:
    explicit PowerModel(QObject *parent = nullptr, const QString &socketPath = QString());

    void start();

public slots:
    void reboot();

signals:
    void rebootFailed();

private:
    void onMessage(const QJsonObject &message);

    DaemonClient mClient;
    int mRebootId = -1;
};

#endif // POWER_MODEL_H
