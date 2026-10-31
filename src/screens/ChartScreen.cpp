#include "screens/ChartScreen.h"
#include "ui_ChartScreen.h"

#include "model/HistoryModel.h"

#include <QButtonGroup>
#include <QDateTime>
#include <QFont>
#include <QPainter>
#include <QScroller>
#include <QScrollerProperties>
#include <QShowEvent>
#include <QVBoxLayout>

#include <QtNumeric>

#include <limits>

static const qint64 kHour = 3600;

ChartScreen::ChartScreen(const HistoryModel *history, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChartScreen)
    , mRangeGroup(new QButtonGroup(this))
    , mHistory(history)
    , mRangeHours(1)
{
    ui->setupUi(this);

    mCharts[Temp] = makeChart(QStringLiteral("Temperature (°C)"), QColor("#e6755b"));
    mCharts[Humi] = makeChart(QStringLiteral("Humidity (%)"), QColor("#4f9ad6"));
    mCharts[Light] = makeChart(QStringLiteral("Light (lx)"), QColor("#e0b341"));

    for (int i = 0; i < MetricCount; ++i) {
        ui->chartsLayout->addWidget(mCharts[i].view);
    }

    QScroller::grabGesture(ui->chartScroll->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(ui->chartScroll->viewport());
    QScrollerProperties props = scroller->scrollerProperties();
    props.setScrollMetric(QScrollerProperties::AxisLockThreshold, 1.0);
    props.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.5);
    props.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0);
    props.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0);
    scroller->setScrollerProperties(props);

    mRangeGroup->setExclusive(true);
    mRangeGroup->addButton(ui->range1hButton, 1);
    mRangeGroup->addButton(ui->range6hButton, 6);
    mRangeGroup->addButton(ui->range24hButton, 24);
    QObject::connect(mRangeGroup, &QButtonGroup::idClicked, this, &ChartScreen::setRangeHours);

    refresh();
}

ChartScreen::~ChartScreen()
{
    delete ui;
}

ChartScreen::ChartItem ChartScreen::makeChart(const QString &title, const QColor &color)
{
    ChartItem item;

    item.series = new QLineSeries();
    QPen pen(color);
    pen.setWidth(2);
    item.series->setPen(pen);

    QFont titleFont;
    titleFont.setPointSize(6);
    titleFont.setBold(true);

    QFont labelFont;
    labelFont.setPointSize(5);

    item.chart = new QChart();
    item.chart->addSeries(item.series);
    item.chart->legend()->hide();
    item.chart->setTitle(title);
    item.chart->setTitleFont(titleFont);
    item.chart->setMargins(QMargins(0, 0, 0, 0));
    item.chart->setContentsMargins(0, 0, 0, 0);
    item.chart->setBackgroundRoundness(6);

    item.axisX = new QDateTimeAxis();
    item.axisX->setTickCount(3);
    item.axisX->setFormat(QStringLiteral("hh:mm"));
    item.axisX->setLabelsFont(labelFont);
    item.chart->addAxis(item.axisX, Qt::AlignBottom);
    item.series->attachAxis(item.axisX);

    item.axisY = new QValueAxis();
    item.axisY->setTickCount(4);
    item.axisY->setLabelFormat(QStringLiteral("%.0f"));
    item.axisY->setLabelsFont(labelFont);
    item.chart->addAxis(item.axisY, Qt::AlignLeft);
    item.series->attachAxis(item.axisY);

    item.view = new QChartView(item.chart);
    item.view->setRenderHint(QPainter::Antialiasing);
    item.view->setMinimumHeight(190);
    item.view->setFrameShape(QFrame::NoFrame);
    item.view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    item.view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    item.view->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    item.view->viewport()->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    return item;
}

void ChartScreen::setRangeHours(int hours)
{
    mRangeHours = hours;
    refresh();
}

void ChartScreen::reload()
{
    // Màn đang ẩn thì để showEvent() vẽ lại khi user mở tab.
    if (isVisible()) {
        refresh();
    }
}

void ChartScreen::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refresh();
}

qint64 ChartScreen::stepForRange(int rangeHours) const
{
    if (rangeHours <= 1) return 120;    // 1h  -> 2 phút/điểm
    if (rangeHours <= 6) return 600;    // 6h  -> 10 phút/điểm
    return 1800;                        // 24h -> 30 phút/điểm
}

void ChartScreen::rebuildSeries()
{
    for (int i = 0; i < MetricCount; ++i) {
        mCharts[i].series->clear();
    }

    if (!mHistory) return;

    const QVector<Sample> buckets = mHistory->range(mRangeHours, (int)stepForRange(mRangeHours));
    for (const Sample &s : buckets) {
        const qreal x = (qreal)(s.timestampSec * 1000);
        const double values[MetricCount] = {s.tempC, s.humidity, s.lux};
        for (int i = 0; i < MetricCount; ++i) {
            if (!qIsNaN(values[i])) {
                mCharts[i].series->append(x, values[i]);
            }
        }
    }
}

void ChartScreen::refresh()
{
    rebuildSeries();

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const qint64 fromMs = nowMs - mRangeHours * kHour * 1000;

    for (int i = 0; i < MetricCount; ++i) {
        ChartItem &c = mCharts[i];
        c.axisX->setRange(QDateTime::fromMSecsSinceEpoch(fromMs),
                          QDateTime::fromMSecsSinceEpoch(nowMs));

        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();
        const QList<QPointF> points = c.series->points();
        for (const QPointF &p : points) {
            if (p.x() < fromMs) continue;
            minY = qMin(minY, p.y());
            maxY = qMax(maxY, p.y());
        }

        if (minY > maxY) {
            minY = 0.0;
            maxY = 1.0;
        }

        double pad = (maxY - minY) * 0.1;
        if (pad < 1.0) pad = 1.0;
        c.axisY->setRange(minY - pad, maxY + pad);
    }
}
