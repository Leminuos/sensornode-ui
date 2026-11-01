#ifndef METRIC_H
#define METRIC_H

#include <QMetaType>

#include "model/Sample.h"

enum class Metric { Temperature, Humidity, Light };

Q_DECLARE_METATYPE(Metric)

inline double metricValue(const Sample &sample, Metric metric)
{
    switch (metric) {
    case Metric::Temperature: return sample.tempC;
    case Metric::Humidity:    return sample.humidity;
    case Metric::Light:       return sample.lux;
    }
    return sample.tempC;
}

#endif // METRIC_H
