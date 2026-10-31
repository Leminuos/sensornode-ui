#ifndef HISTORY_MODEL_H
#define HISTORY_MODEL_H

#include <QObject>
#include <QString>
#include <QVector>

#include "model/Sample.h"

struct sqlite3;

class HistoryModel : public QObject
{
    Q_OBJECT

public:
    explicit HistoryModel(QObject *parent = nullptr, const QString &dbPath = QString());
    ~HistoryModel();

    QVector<Sample> range(int hours, int bucketSec = 0) const;

private:
    bool ensureOpen() const;
    void close() const;

    QString mPath;
    mutable sqlite3 *mDb = nullptr;
};

#endif // HISTORY_MODEL_H
