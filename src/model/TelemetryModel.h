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
    bool isMqttConnected() const { return mMqttConnected; }

    bool setInterval(int intervalMs);

signals:
    void sampleReceived(double tempC, double humidity, double lux);
    void reportReceived(const SensorReport &report);
    void macAddressChanged(const QString &mac);
    void nodeIdChanged(const QString &nodeId);
    void onlineChanged(bool online);
    void intervalChanged(int intervalMs);
    void staleChanged(bool stale);
    void mqttConnectedChanged(bool connected);
    void intervalApplied(bool ok);

private:
    void onMessage(const QJsonObject &message);
    void emitSample(const QJsonObject &sample);
    void setStale(bool stale);
    void armStaleTimer();
    void updateInterval(const QJsonObject &message);
    void setMqttConnected(bool connected);

    DaemonClient mClient;
    int mIntervalMs = 5000;
    bool mStale = true;
    bool mMqttConnected = false;
    int mIntervalId = -1;
    QTimer mStaleTimer;
};

#endif // TELEMETRY_MODEL_H
