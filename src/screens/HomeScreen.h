#ifndef HOME_SCREEN_H
#define HOME_SCREEN_H

#include <QWidget>

#include "model/Metric.h"

class HistoryModel;
class MetricTile;

class HomeScreen : public QWidget
{
    Q_OBJECT

public:
    explicit HomeScreen(const HistoryModel *history, QWidget *parent = nullptr);

public slots:
    void setValues(double tempC, double humidity, double lux);
    void setStale(bool stale);

signals:
    void metricRequested(Metric metric);

protected:
    void showEvent(QShowEvent *event) override;

private:
    void applyReadings();
    void reloadTrends();

    const HistoryModel *mHistory;
    MetricTile *mTemperature;
    MetricTile *mHumidity;
    MetricTile *mLight;

    double mTempC;
    double mHumidityValue;
    double mLux;
    bool mStale = true;
    qint64 mTrendLoadedAtSec = 0;
};

#endif // HOME_SCREEN_H
