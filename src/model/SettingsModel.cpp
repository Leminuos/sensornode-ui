#include "model/SettingsModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QJsonArray>

#include <QDebug>

static const QString kBrightness = QStringLiteral("brightness");
static const QString kAutoMode = QStringLiteral("auto_mode");
static const QString kAutoDim = QStringLiteral("auto_dim_min");
static const QString kSamplingPeriod = QStringLiteral("sampling_period_s");
static const QString kTemperatureUnit = QStringLiteral("temperature_unit");
static const QString kLanguage = QStringLiteral("language");
static const QString kLocation = QStringLiteral("location");
static const QString kTimeZone = QStringLiteral("timezone");
static const QString kSource = QStringLiteral("hmi");

SettingsModel::SettingsModel(QObject *parent, const QString &socketPath)
    : QObject(parent)
    , mClient(socketPath.isEmpty() ? DaemonEndpoints::settingsSocket() : socketPath)
{
    connect(&mClient, &DaemonClient::disconnected, this, [this]() {
        if (mResetId >= 0) {
            mResetId = -1;
            emit resetFinished(false);
        }
        emit availableChanged(false);
    });
    connect(&mClient, &DaemonClient::messageReceived, this, &SettingsModel::onMessage);
}

void SettingsModel::start()
{
    mClient.start();
}

void SettingsModel::setBrightness(int value)
{
    set(kBrightness, value);
}

void SettingsModel::setAutoMode(bool autoMode)
{
    if (autoMode == mAutoMode) return;

    mAutoMode = autoMode;
    set(kAutoMode, autoMode);
    emit autoModeChanged(autoMode);
}

bool SettingsModel::setAutoDim(int minutes)
{
    return set(kAutoDim, minutes);
}

bool SettingsModel::setSamplingPeriod(int seconds)
{
    mSamplingPeriodSec = seconds;
    return set(kSamplingPeriod, seconds);
}

bool SettingsModel::setTemperatureUnit(TemperatureUnit unit)
{
    return set(kTemperatureUnit, unit == TemperatureUnit::Fahrenheit ? QStringLiteral("F") : QStringLiteral("C"));
}

bool SettingsModel::setLanguage(const QString &code)
{
    return set(kLanguage, code);
}

bool SettingsModel::setLocation(const QString &id, const QString &timeZone)
{
    return set(kLocation, id) && set(kTimeZone, timeZone);
}

bool SettingsModel::reset()
{
    if (mResetId >= 0) return true;

    mResetId = mClient.sendCommand(QStringLiteral("reset"), {{QStringLiteral("source"), kSource}});
    return mResetId >= 0;
}

bool SettingsModel::set(const QString &key, const QJsonValue &value)
{
    return mClient.sendCommand(QStringLiteral("set"), {{QStringLiteral("key"), key},
                                                       {QStringLiteral("value"), value},
                                                       {QStringLiteral("source"), kSource}}) >= 0;
}

void SettingsModel::applySettings(const QJsonObject &settings, const QStringList &keys)
{
    const auto has = [&keys](const QString &key) { return keys.isEmpty() || keys.contains(key); };

    if (has(kBrightness)) emit brightnessChanged(settings.value(kBrightness).toInt());
    if (has(kAutoMode)) {
        mAutoMode = settings.value(kAutoMode).toBool();
        emit autoModeChanged(mAutoMode);
    }
    if (has(kAutoDim)) emit autoDimChanged(settings.value(kAutoDim).toInt());
    if (has(kSamplingPeriod)) mSamplingPeriodSec = settings.value(kSamplingPeriod).toInt(mSamplingPeriodSec);
    if (has(kTemperatureUnit)) {
        emit temperatureUnitChanged(settings.value(kTemperatureUnit).toString() == QLatin1String("F")
                                        ? TemperatureUnit::Fahrenheit
                                        : TemperatureUnit::Celsius);
    }
    if (has(kLanguage)) emit languageChanged(settings.value(kLanguage).toString());
    if (has(kLocation)) emit locationChanged(settings.value(kLocation).toString());
}

void SettingsModel::onMessage(const QJsonObject &message)
{
    const QString type = message.value(QStringLiteral("type")).toString();

    if (type == QLatin1String("hello")) {
        applySettings(message.value(QStringLiteral("settings")).toObject());
        emit availableChanged(true);
        return;
    }
    // Thay đổi từ web hoặc client khác; thay đổi của chính HMI đã được áp dụng lúc gửi.
    if (type == QLatin1String("changed")) {
        if (message.value(QStringLiteral("source")).toString() == kSource) return;
        QStringList keys;
        for (const QJsonValue &k : message.value(QStringLiteral("keys")).toArray()) keys << k.toString();
        if (!keys.isEmpty()) applySettings(message.value(QStringLiteral("settings")).toObject(), keys);
        return;
    }
    if (type != QLatin1String("reply")) return;

    const bool ok = message.value(QStringLiteral("ok")).toBool();
    if (mResetId >= 0 && message.value(QStringLiteral("id")).toInt(-1) == mResetId) {
        mResetId = -1;
        if (ok) applySettings(message.value(QStringLiteral("settings")).toObject());
        emit resetFinished(ok);
    }
    if (!ok) {
        qWarning() << "settingsd rejected" << message.value(QStringLiteral("error")).toString();
    }
}
