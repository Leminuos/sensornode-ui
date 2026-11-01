#ifndef DEMO_DATA_H
#define DEMO_DATA_H

#include "model/Metric.h"

namespace DemoData {

constexpr int brightness = 72;
constexpr int samplingPeriodSec = 5;
constexpr int autoDimMinutes = 5;
constexpr int locationIndex = 0;
constexpr TemperatureUnit temperatureUnit = TemperatureUnit::Celsius;
constexpr const char *language = "vi";
constexpr bool mqttConnected = true;

} // namespace DemoData

#endif // DEMO_DATA_H
