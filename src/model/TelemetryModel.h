#ifndef TELEMETRY_MODEL_H
#define TELEMETRY_MODEL_H

#include <QObject>
#include <QString>

#include "ipc/DaemonClient.h"

class TelemetryModel : public QObject
{
    Q_OBJECT

public:
    explicit TelemetryModel(QObject *parent = nullptr, const QString &socketPath = QString());

    void start();
    bool isOnline() const;

signals:
    void sampleReceived(double tempC, double humidity, double lux);
    void macAddressChanged(const QString &mac);
    void onlineChanged(bool online);

private:
    void onMessage(const QJsonObject &message);
    void emitSample(const QJsonObject &sample);

    DaemonClient mClient;
};

#endif // TELEMETRY_MODEL_H
