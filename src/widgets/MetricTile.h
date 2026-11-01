#ifndef METRIC_TILE_H
#define METRIC_TILE_H

#include <QAbstractButton>
#include <QPainterPath>
#include <QPointF>
#include <QVector>

#include "model/Metric.h"

class MetricTile : public QAbstractButton
{
    Q_OBJECT

public:
    MetricTile(Metric metric, const QString &label, QWidget *parent = nullptr);

    void setVisualGeometry(const QRect &rect);
    void setReading(const QString &value);
    QString reading() const { return mValue; }

    void setTrend(const QVector<QPointF> &points);
    bool hasTrend() const { return !mLine.isEmpty(); }

protected:
    void paintEvent(QPaintEvent *event) override;
    bool hitButton(const QPoint &pos) const override;

private:
    QRect faceRect() const;
    void rebuildTrend();

    Metric mMetric;
    QString mValue;
    QVector<QPointF> mTrend;
    QPainterPath mLine;
    QPainterPath mFill;
};

#endif // METRIC_TILE_H
