#ifndef SETTINGS_MODEL_H
#define SETTINGS_MODEL_H

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QString>

#include "ipc/DaemonClient.h"

class SettingsModel : public QObject
{
    Q_OBJECT

public:
    explicit SettingsModel(QObject *parent = nullptr, const QString &socketPath = QString());

    void start();

public slots:
    void setBrightness(int value);
    void setAutoMode(bool autoMode);

signals:
    void brightnessChanged(int value);
    void autoModeChanged(bool autoMode);
    void availableChanged(bool available);

private:
    void onMessage(const QJsonObject &message);
    void set(const QString &key, const QJsonValue &value);

    DaemonClient mClient;
    bool mAutoMode = false;
};

#endif // SETTINGS_MODEL_H
