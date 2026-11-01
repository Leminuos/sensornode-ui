#ifndef STUBS_H
#define STUBS_H

#include <QString>

#include "model/Metric.h"

namespace Stubs {

bool setSamplingPeriod(int seconds);
bool setLocation(const QString &city, const QString &timeZone);
void setTemperatureUnit(TemperatureUnit unit);
void setAutoDim(int minutes);
void setLanguage(const QString &language);
void restartDevice();
void applyTouchCalibration(const QString &pointercal);
void keepTouchCalibration();
void restoreTouchCalibration();
QString rolledBackVersion();

} // namespace Stubs

#endif // STUBS_H
