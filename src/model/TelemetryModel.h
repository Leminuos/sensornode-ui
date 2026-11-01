#ifndef TELEMETRY_MODEL_H
#define TELEMETRY_MODEL_H

#include <QObject>
#include <QString>
#include <QTimer>

#include "ipc/DaemonClient.h"
#include "model/Sample.h"

class TelemetryModel : public QObject
{
    Q_OBJECT

public:
    explicit TelemetryModel(QObject *parent = nullptr, const QString &socketPath = QString());

    void start();
    bool isOnline() const;
    bool isStale() const { return mStale; }
    int intervalMs() const { return mIntervalMs; }

signals:
    void sampleReceived(double tempC, double humidity, double lux);
    void reportReceived(const SensorReport &report);
    void macAddressChanged(const QString &mac);
    void onlineChanged(bool online);
    void intervalChanged(int intervalMs);
    void staleChanged(bool stale);

private:
    void onMessage(const QJsonObject &message);
    void emitSample(const QJsonObject &sample);
    void setStale(bool stale);
    void armStaleTimer();

    DaemonClient mClient;
    int mIntervalMs = 5000;
    bool mStale = true;
    QTimer mStaleTimer;
};

#endif // TELEMETRY_MODEL_H
