#include "app/Stubs.h"

#include <QDebug>

bool Stubs::setSamplingPeriod(int seconds)
{
    qInfo("[STUB] set sampling period: %d s", seconds);
    return true;
}

void Stubs::restartDevice()
{
    qInfo("[STUB] restart device");
}

QString Stubs::rolledBackVersion()
{
    qInfo("[STUB] check rollback: none");
    return QString();
}

void Stubs::applyTouchCalibration(const QString &pointercal)
{
    qInfo("[STUB] apply touch calibration: %s", qUtf8Printable(pointercal));
}

void Stubs::keepTouchCalibration()
{
    qInfo("[STUB] keep touch calibration");
}

void Stubs::restoreTouchCalibration()
{
    qInfo("[STUB] restore touch calibration");
}
