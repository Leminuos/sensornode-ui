#ifndef SAMPLE_H
#define SAMPLE_H

#include <QMetaType>
#include <QtGlobal>

struct Sample {
    qint64 timestampSec;  // epoch giây
    double tempC;         // °C
    double humidity;      // %
    double lux;           // lx
};

Q_DECLARE_METATYPE(Sample)

#endif // SAMPLE_H
