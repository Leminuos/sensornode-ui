#include "model/HistoryModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QDateTime>
#include <QDebug>
#include <QtNumeric>

#include <sqlite3.h>

static const char *kRawSql =
    "SELECT ts, temp_c, humi_rh, lux FROM samples"
    " WHERE ts >= ?1 AND ts <= ?2 ORDER BY ts";

static const char *kBucketSql =
    "SELECT ts / ?3 * ?3 AS b, AVG(temp_c), AVG(humi_rh), AVG(lux) FROM samples"
    " WHERE ts >= ?1 AND ts <= ?2 GROUP BY b ORDER BY b";

static double column(sqlite3_stmt *stmt, int col)
{
    return sqlite3_column_type(stmt, col) == SQLITE_NULL ? qQNaN() : sqlite3_column_double(stmt, col);
}

HistoryModel::HistoryModel(QObject *parent, const QString &dbPath)
    : QObject(parent)
    , mPath(dbPath.isEmpty() ? DaemonEndpoints::telemetryDb() : dbPath)
{
}

HistoryModel::~HistoryModel()
{
    close();
}

bool HistoryModel::ensureOpen() const
{
    if (mDb) return true;

    if (sqlite3_open_v2(mPath.toUtf8().constData(), &mDb, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        qWarning() << "HistoryModel: cannot open" << mPath << sqlite3_errmsg(mDb);
        close();
        return false;
    }

    sqlite3_busy_timeout(mDb, 200);
    return true;
}

void HistoryModel::close() const
{
    sqlite3_close(mDb);
    mDb = nullptr;
}

QVector<Sample> HistoryModel::range(int hours, int bucketSec) const
{
    QVector<Sample> out;
    if (!ensureOpen()) return out;

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const qint64 fromMs = nowMs - (qint64)hours * 3600 * 1000;

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(mDb, bucketSec > 0 ? kBucketSql : kRawSql, -1, &stmt, nullptr) != SQLITE_OK) {
        qWarning() << "HistoryModel: query failed" << sqlite3_errmsg(mDb);
        close();
        return out;
    }

    sqlite3_bind_int64(stmt, 1, fromMs);
    sqlite3_bind_int64(stmt, 2, nowMs);
    if (bucketSec > 0) {
        sqlite3_bind_int64(stmt, 3, (qint64)bucketSec * 1000);
    }

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        out.append(Sample{sqlite3_column_int64(stmt, 0) / 1000, column(stmt, 1), column(stmt, 2), column(stmt, 3)});
    }
    
    if (rc != SQLITE_DONE) {
        qWarning() << "HistoryModel: read failed" << sqlite3_errmsg(mDb);
    }

    sqlite3_finalize(stmt);
    return out;
}
