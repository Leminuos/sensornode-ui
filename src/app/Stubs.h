#ifndef STUBS_H
#define STUBS_H

#include <QString>

namespace Stubs {

void applyTouchCalibration(const QString &pointercal);
void keepTouchCalibration();
void restoreTouchCalibration();

} // namespace Stubs

#endif // STUBS_H
