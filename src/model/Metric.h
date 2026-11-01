#ifndef METRIC_H
#define METRIC_H

#include <QMetaType>

#include "model/Sample.h"

enum class Metric { Temperature, Humidity, Light };

enum class TemperatureUnit { Celsius, Fahrenheit };

Q_DECLARE_METATYPE(Metric)
Q_DECLARE_METATYPE(TemperatureUnit)

inline double toFahrenheit(double celsius)
{
    return celsius * 9.0 / 5.0 + 32.0;
}

inline double displayTemperature(double celsius, TemperatureUnit unit)
{
    return unit == TemperatureUnit::Fahrenheit ? toFahrenheit(celsius) : celsius;
}

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
