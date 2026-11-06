#ifndef SETTINGS_MODEL_H
#define SETTINGS_MODEL_H

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QString>

#include "ipc/DaemonClient.h"
#include "model/Metric.h"

class SettingsModel : public QObject
{
    Q_OBJECT

public:
    explicit SettingsModel(QObject *parent = nullptr, const QString &socketPath = QString());

    void start();
    int samplingPeriod() const { return mSamplingPeriodSec; }

    // Các setter trả về false nếu chưa kết nối được settingsd.
    bool setAutoDim(int minutes);
    bool setSamplingPeriod(int seconds);
    bool setTemperatureUnit(TemperatureUnit unit);
    bool setLanguage(const QString &code);
    bool setLocation(const QString &id, const QString &timeZone);
    bool reset();

public slots:
    void setBrightness(int value);
    void setAutoMode(bool autoMode);

signals:
    void brightnessChanged(int value);
    void autoModeChanged(bool autoMode);
    void autoDimChanged(int minutes);
    void temperatureUnitChanged(TemperatureUnit unit);
    void languageChanged(const QString &code);
    void locationChanged(const QString &id);
    void availableChanged(bool available);
    void resetFinished(bool ok);

private:
    void onMessage(const QJsonObject &message);
    void applySettings(const QJsonObject &settings);
    bool set(const QString &key, const QJsonValue &value);

    DaemonClient mClient;
    bool mAutoMode = false;
    int mSamplingPeriodSec = 5;
    int mResetId = -1;
};

#endif // SETTINGS_MODEL_H
