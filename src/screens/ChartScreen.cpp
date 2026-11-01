#include "screens/ChartScreen.h"

#include "app/Theme.h"
#include "model/ChartScale.h"
#include "model/HistoryModel.h"
#include "widgets/SegmentedControl.h"

#include <QButtonGroup>
#include <QDateTime>
#include <QFrame>
#include <QGraphicsLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QShowEvent>
#include <QVariant>

#include <QtCharts/QAreaSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <QtNumeric>

QT_CHARTS_USE_NAMESPACE

static constexpr int kCanvasWidth = 212;
static constexpr int kCanvasHeight = 102;
static constexpr int kPlotTop = 5;
static constexpr int kPlotBottom = 87;
static constexpr int kPlotRight = 208;
static constexpr int kMinPlotLeft = 22;
static constexpr int kAxisGap = 4;
static constexpr int kYIntervals = 3;

static const struct { int hours; int bucketSec; } kRanges[] = {{1, 120}, {6, 600}, {24, 1800}};

static int decimalsFor(Metric metric)
{
    return metric == Metric::Temperature ? 1 : 0;
}

class ChartCanvas : public QChartView
{
    Q_OBJECT

public:
    explicit ChartCanvas(QWidget *parent)
        : QChartView(new QChart, parent)
        , mLine(new QLineSeries)
        , mAreaUpper(new QLineSeries)
        , mArea(new QAreaSeries(mAreaUpper))
        , mAxisX(new QDateTimeAxis)
        , mAxisY(new QValueAxis)
    {
        QChart *c = chart();
        c->legend()->hide();
        c->setBackgroundVisible(false);
        c->setMargins(QMargins(0, 0, 0, 0));
        c->layout()->setContentsMargins(0, 0, 0, 0);

        c->addSeries(mArea);
        c->addSeries(mLine);

        mAxisX->setLabelsVisible(false);
        mAxisX->setGridLineVisible(false);
        mAxisX->setLineVisible(false);
        c->addAxis(mAxisX, Qt::AlignBottom);

        mAxisY->setLabelsVisible(false);
        mAxisY->setLineVisible(false);
        mAxisY->setTickCount(kYIntervals + 1);
        mAxisY->setGridLinePen(QPen(Theme::color(QStringLiteral("divider")), 1));
        c->addAxis(mAxisY, Qt::AlignLeft);

        for (QAbstractSeries *series : {static_cast<QAbstractSeries *>(mArea), static_cast<QAbstractSeries *>(mLine)}) {
            series->attachAxis(mAxisX);
            series->attachAxis(mAxisY);
        }

        setRenderHint(QPainter::Antialiasing);
        setFrameShape(QFrame::NoFrame);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        viewport()->setAttribute(Qt::WA_TransparentForMouseEvents);
        setFixedSize(kCanvasWidth, kCanvasHeight);
    }

    void setMetric(Metric metric)
    {
        const char *line = metric == Metric::Temperature ? "chart-temp"
                         : metric == Metric::Humidity    ? "chart-humi"
                                                         : "chart-light";
        const QString name = QLatin1String(line);
        mLine->setPen(QPen(Theme::color(name), 1.5));
        mArea->setPen(Qt::NoPen);
        mArea->setBrush(Theme::color(name + QStringLiteral("-fill")));
    }

    void setData(const QVector<QPointF> &points, qint64 fromMs, qint64 toMs)
    {
        mEmpty = points.isEmpty();
        mFromMs = fromMs;
        mToMs = toMs;

        mAxisX->setRange(QDateTime::fromMSecsSinceEpoch(fromMs), QDateTime::fromMSecsSinceEpoch(toMs));

        double low = 0;
        double high = 1;
        if (!mEmpty) {
            low = high = points.first().y();
            for (const QPointF &point : points) {
                low = qMin(low, point.y());
                high = qMax(high, point.y());
            }
        }
        mScale = niceScale(low, high);
        mAxisY->setRange(mScale.min, mScale.max);
        mAxisY->setGridLineVisible(!mEmpty);

        // QAreaSeries tô tới y = 0 nếu không có đường dưới, nên phải kẹp đáy vào đáy biểu đồ.
        QVector<QPointF> area = points;
        if (!mEmpty) {
            area.prepend(QPointF(points.first().x(), mScale.min));
            area.append(QPointF(points.last().x(), mScale.min));
        }
        mAreaUpper->replace(area);
        mLine->replace(points);

        const QFontMetrics metrics(Theme::font(QStringLiteral("caption")));
        int widest = 0;
        for (int i = 0; i <= kYIntervals; ++i) {
            widest = qMax(widest, metrics.horizontalAdvance(axisLabel(i)));
        }
        mPlotLeft = qMax(kMinPlotLeft, widest + kAxisGap);
        chart()->setPlotArea(QRectF(mPlotLeft, kPlotTop, kPlotRight - mPlotLeft, kPlotBottom - kPlotTop));

        viewport()->update();
    }

    bool isEmpty() const { return mEmpty; }

protected:
    void drawForeground(QPainter *painter, const QRectF &) override
    {
        painter->setFont(Theme::font(QStringLiteral("caption")));

        if (mEmpty) {
            painter->setPen(Theme::color(QStringLiteral("text-muted")));
            painter->drawText(QRect(0, kPlotTop, kCanvasWidth, kPlotBottom - kPlotTop),
                              Qt::AlignCenter | Qt::TextWordWrap, tr("Chưa có dữ liệu trong khoảng này"));
            return;
        }

        painter->setPen(Theme::color(QStringLiteral("text-muted")));

        const int plotHeight = kPlotBottom - kPlotTop;
        for (int i = 0; i <= kYIntervals; ++i) {
            const int y = kPlotBottom - plotHeight * i / kYIntervals;
            painter->drawText(QRect(0, y - 8, mPlotLeft - kAxisGap, 16), Qt::AlignRight | Qt::AlignVCenter, axisLabel(i));
        }

        const QRect row(mPlotLeft, kCanvasHeight - 15, kPlotRight - mPlotLeft, 15);
        const qint64 middleMs = (mFromMs + mToMs) / 2;
        painter->drawText(row, Qt::AlignLeft | Qt::AlignBottom, timeLabel(mFromMs));
        painter->drawText(row, Qt::AlignHCenter | Qt::AlignBottom, timeLabel(middleMs));
        painter->drawText(row, Qt::AlignRight | Qt::AlignBottom, timeLabel(mToMs));
    }

private:
    QString axisLabel(int tick) const
    {
        return QString::number(mScale.min + tick * mScale.step, 'f', 0);
    }

    static QString timeLabel(qint64 ms)
    {
        return QDateTime::fromMSecsSinceEpoch(ms).toString(QStringLiteral("HH:mm"));
    }

    QLineSeries *mLine;
    QLineSeries *mAreaUpper;
    QAreaSeries *mArea;
    QDateTimeAxis *mAxisX;
    QValueAxis *mAxisY;
    ChartScale mScale{0, 3, 1};
    int mPlotLeft = kMinPlotLeft;
    qint64 mFromMs = 0;
    qint64 mToMs = 0;
    bool mEmpty = true;
};

ChartScreen::ChartScreen(const HistoryModel *history, QWidget *parent)
    : QWidget(parent)
    , mHistory(history)
    , mMetricTabs(new QButtonGroup(this))
    , mRange(new SegmentedControl({QStringLiteral("1h"), QStringLiteral("6h"), QStringLiteral("24h")}, this))
{
    auto *tabLine = new QFrame(this);
    tabLine->setObjectName(QStringLiteral("metricTabLine"));
    tabLine->setGeometry(8, 41, 224, 1);

    const struct { Metric metric; const char *property; QString label; int x; int width; } tabs[] = {
        {Metric::Temperature, "temperature", tr("Nhiệt độ"), 8, 75},
        {Metric::Humidity, "humidity", tr("Độ ẩm"), 83, 74},
        {Metric::Light, "light", tr("Ánh sáng"), 157, 75},
    };
    for (const auto &tab : tabs) {
        auto *button = new QPushButton(tab.label, this);
        button->setCheckable(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setProperty("metricTab", tab.property);
        button->setGeometry(tab.x, 6, tab.width, 36);
        mMetricTabs->addButton(button, static_cast<int>(tab.metric));
    }
    mMetricTabs->button(static_cast<int>(mMetric))->setChecked(true);
    connect(mMetricTabs, &QButtonGroup::idClicked, this, [this](int id) {
        showMetric(static_cast<Metric>(id));
    });

    mRange->setGeometry(8, 46, 224, 36);
    connect(mRange, &SegmentedControl::currentChanged, this, [this]() { refresh(); });

    auto *card = new QFrame(this);
    card->setProperty("surface", "card");
    card->setGeometry(QRect(8, 86, 224, 154).marginsAdded(Theme::shadowMargins()));

    const QPoint face(Theme::shadowMargins().left(), Theme::shadowMargins().top());

    mCanvas = new ChartCanvas(card);
    mCanvas->move(face + QPoint(6, 6));

    const struct { QString caption; QLabel *&value; int x; int width; } stats[] = {
        {tr("Thấp nhất"), mMin, 6, 71},
        {tr("Trung bình"), mAverage, 77, 70},
        {tr("Cao nhất"), mMax, 147, 71},
    };
    for (const auto &stat : stats) {
        auto *caption = new QLabel(stat.caption, card);
        caption->setObjectName(QStringLiteral("statCaption"));
        caption->setAlignment(Qt::AlignCenter);
        caption->setGeometry(face.x() + stat.x, face.y() + 110, stat.width, 15);

        stat.value = new QLabel(card);
        stat.value->setObjectName(QStringLiteral("statValue"));
        stat.value->setAlignment(Qt::AlignCenter);
        stat.value->setGeometry(face.x() + stat.x, face.y() + 125, stat.width, 19);
    }

    mCanvas->setMetric(mMetric);
}

void ChartScreen::showMetric(Metric metric)
{
    mMetric = metric;
    mMetricTabs->button(static_cast<int>(metric))->setChecked(true);
    mCanvas->setMetric(metric);
    if (isVisible()) {
        refresh();
    }
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

void ChartScreen::refresh()
{
    const auto &range = kRanges[mRange->currentIndex()];

    QVector<QPointF> points;
    RangeStats stats{qQNaN(), qQNaN(), qQNaN()};

    if (mHistory) {
        const QVector<Sample> buckets = mHistory->range(range.hours, range.bucketSec);
        for (const Sample &sample : buckets) {
            const double value = metricValue(sample, mMetric);
            if (!qIsNaN(value)) {
                points.append(QPointF(sample.timestampSec * 1000.0, value));
            }
        }
        stats = mHistory->stats(range.hours, mMetric);
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    mCanvas->setData(points, nowMs - range.hours * 3600 * 1000LL, nowMs);

    const auto format = [this](double value) {
        return qIsNaN(value) ? QStringLiteral("--") : QString::number(value, 'f', decimalsFor(mMetric));
    };
    mMin->setText(format(stats.min));
    mAverage->setText(format(stats.average));
    mMax->setText(format(stats.max));
}

#include "ChartScreen.moc"
