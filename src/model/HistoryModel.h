#ifndef HISTORY_MODEL_H
#define HISTORY_MODEL_H

#include <QObject>
#include <QString>
#include <QVector>

#include "model/Metric.h"
#include "model/Sample.h"

struct sqlite3;

struct RangeStats
{
    double min;
    double average;
    double max;
};

class HistoryModel : public QObject
{
    Q_OBJECT

public:
    explicit HistoryModel(QObject *parent = nullptr, const QString &dbPath = QString());
    ~HistoryModel();

    QVector<Sample> range(int hours, int bucketSec = 0) const;
    RangeStats stats(int hours, Metric metric) const;

private:
    bool ensureOpen() const;
    void close() const;

    QString mPath;
    mutable sqlite3 *mDb = nullptr;
};

#endif // HISTORY_MODEL_H
