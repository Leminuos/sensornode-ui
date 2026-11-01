#include "screens/HomeScreen.h"

#include "model/HistoryModel.h"
#include "widgets/MetricTile.h"

#include <QDateTime>
#include <QShowEvent>
#include <QtNumeric>

static constexpr int kTrendHours = 1;
static constexpr int kTrendBucketSec = 120;

static QString reading(double value, int decimals)
{
    return qIsNaN(value) ? QString() : QString::number(value, 'f', decimals);
}

HomeScreen::HomeScreen(const HistoryModel *history, QWidget *parent)
    : QWidget(parent)
    , mHistory(history)
    , mTemperature(new MetricTile(Metric::Temperature, tr("Nhiệt độ"), this))
    , mHumidity(new MetricTile(Metric::Humidity, tr("Độ ẩm"), this))
    , mLight(new MetricTile(Metric::Light, tr("Ánh sáng"), this))
    , mTempC(qQNaN())
    , mHumidityValue(qQNaN())
    , mLux(qQNaN())
{
    mTemperature->setVisualGeometry(QRect(8, 6, 224, 128));
    mHumidity->setVisualGeometry(QRect(8, 141, 108, 99));
    mLight->setVisualGeometry(QRect(124, 141, 108, 99));

    connect(mTemperature, &QAbstractButton::clicked, this, [this]() { emit metricRequested(Metric::Temperature); });
    connect(mHumidity, &QAbstractButton::clicked, this, [this]() { emit metricRequested(Metric::Humidity); });
    connect(mLight, &QAbstractButton::clicked, this, [this]() { emit metricRequested(Metric::Light); });
}

void HomeScreen::setValues(double tempC, double humidity, double lux)
{
    mTempC = tempC;
    mHumidityValue = humidity;
    mLux = lux;
    applyReadings();

    // Mỗi điểm đồ thị là 2 phút nên không cần đọc DB theo từng mẫu.
    if (isVisible() && QDateTime::currentSecsSinceEpoch() - mTrendLoadedAtSec >= kTrendBucketSec / 2) {
        reloadTrends();
    }
}

void HomeScreen::setStale(bool stale)
{
    mStale = stale;
    applyReadings();
}

void HomeScreen::setTemperatureUnit(TemperatureUnit unit)
{
    mUnit = unit;
    mTemperature->setUnit(unit == TemperatureUnit::Fahrenheit ? QStringLiteral("°F") : QStringLiteral("°C"));
    applyReadings();
}

void HomeScreen::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    reloadTrends();
}

void HomeScreen::applyReadings()
{
    mTemperature->setReading(mStale ? QString() : reading(displayTemperature(mTempC, mUnit), 1));
    mHumidity->setReading(mStale ? QString() : reading(mHumidityValue, 0));
    mLight->setReading(mStale ? QString() : reading(mLux, 0));
}

void HomeScreen::reloadTrends()
{
    if (!mHistory) return;

    mTrendLoadedAtSec = QDateTime::currentSecsSinceEpoch();

    const QVector<Sample> buckets = mHistory->range(kTrendHours, kTrendBucketSec);
    if (buckets.isEmpty()) {
        mTemperature->setTrend({});
        mHumidity->setTrend({});
        mLight->setTrend({});
        return;
    }

    const double newest = buckets.last().timestampSec;
    const double span = kTrendHours * 3600 - kTrendBucketSec;

    QVector<QPointF> temperature, humidity, light;
    for (const Sample &sample : buckets) {
        const double x = qMax(0.0, 1.0 - (newest - sample.timestampSec) / span);
        if (!qIsNaN(sample.tempC)) temperature.append({x, sample.tempC});
        if (!qIsNaN(sample.humidity)) humidity.append({x, sample.humidity});
        if (!qIsNaN(sample.lux)) light.append({x, sample.lux});
    }

    mTemperature->setTrend(temperature);
    mHumidity->setTrend(humidity);
    mLight->setTrend(light);
}
