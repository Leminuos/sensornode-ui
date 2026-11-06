#ifndef STUBS_H
#define STUBS_H

#include <QString>

namespace Stubs {

void restartDevice();
void applyTouchCalibration(const QString &pointercal);
void keepTouchCalibration();
void restoreTouchCalibration();
QString rolledBackVersion();

} // namespace Stubs

#endif // STUBS_H
