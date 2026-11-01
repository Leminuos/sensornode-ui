#include "app/Stubs.h"

#include <QDebug>

bool Stubs::setSamplingPeriod(int seconds)
{
    qInfo("[STUB] set sampling period: %d s", seconds);
    return true;
}

bool Stubs::setLocation(const QString &city, const QString &timeZone)
{
    qInfo("[STUB] set location: %s (%s)", qUtf8Printable(city), qUtf8Printable(timeZone));
    return true;
}

void Stubs::setTemperatureUnit(TemperatureUnit unit)
{
    qInfo("[STUB] set temperature unit: %s", unit == TemperatureUnit::Fahrenheit ? "F" : "C");
}

void Stubs::setAutoDim(int minutes)
{
    qInfo("[STUB] set auto-dim: %d min", minutes);
}

void Stubs::setLanguage(const QString &language)
{
    qInfo("[STUB] set language: %s", qUtf8Printable(language));
}

void Stubs::restartDevice()
{
    qInfo("[STUB] restart device");
}
