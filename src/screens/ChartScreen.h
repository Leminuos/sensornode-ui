#ifndef CHART_SCREEN_H
#define CHART_SCREEN_H

#include <QWidget>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>


QT_BEGIN_NAMESPACE
namespace Ui { class ChartScreen; }
QT_END_NAMESPACE

class HistoryModel;
class QButtonGroup;

QT_CHARTS_USE_NAMESPACE

class ChartScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ChartScreen(const HistoryModel *history, QWidget *parent = nullptr);
    ~ChartScreen();

public slots:
    void reload();

protected:
    void showEvent(QShowEvent *event) override;

private:
    enum Metric { Temp = 0, Humi = 1, Light = 2, MetricCount = 3 };

    struct ChartItem
    {
        QChart *chart;
        QLineSeries *series;
        QDateTimeAxis *axisX;
        QValueAxis *axisY;
        QChartView *view;
    };

    ChartItem makeChart(const QString &title, const QColor &color);
    void setRangeHours(int hours);
    qint64 stepForRange(int rangeHours) const;
    void rebuildSeries();
    void refresh();

    Ui::ChartScreen *ui;
    QButtonGroup *mRangeGroup;
    ChartItem mCharts[MetricCount];
    const HistoryModel *mHistory;
    int mRangeHours;
};

#endif // CHART_SCREEN_H
