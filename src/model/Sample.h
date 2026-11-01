#ifndef SAMPLE_H
#define SAMPLE_H

#include <QMetaType>
#include <QString>
#include <QtGlobal>

struct Sample {
    qint64 timestampSec;  // epoch giây
    double tempC;         // °C
    double humidity;      // %
    double lux;           // lx
};

struct SensorReport
{
    qint64 timestampMs = 0;
    double tempC = 0;
    double humidity = 0;
    double lux = 0;
    QString sht30Error;
    QString bh1750Error;
};

Q_DECLARE_METATYPE(Sample)
Q_DECLARE_METATYPE(SensorReport)

#endif // SAMPLE_H
