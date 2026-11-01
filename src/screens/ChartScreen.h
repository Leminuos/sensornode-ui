#ifndef CHART_SCREEN_H
#define CHART_SCREEN_H

#include <QWidget>

#include "model/Metric.h"

class ChartCanvas;
class HistoryModel;
class QButtonGroup;
class QLabel;
class SegmentedControl;

class ChartScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ChartScreen(const HistoryModel *history, QWidget *parent = nullptr);

public slots:
    void showMetric(Metric metric);
    void setTemperatureUnit(TemperatureUnit unit);
    void reload();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void refresh();
    double displayValue(double value) const;

    const HistoryModel *mHistory;
    QButtonGroup *mMetricTabs;
    SegmentedControl *mRange;
    ChartCanvas *mCanvas;
    QLabel *mMin;
    QLabel *mAverage;
    QLabel *mMax;
    Metric mMetric = Metric::Temperature;
    TemperatureUnit mUnit = TemperatureUnit::Celsius;
};

#endif // CHART_SCREEN_H
