#ifndef DAEMON_ENDPOINTS_H
#define DAEMON_ENDPOINTS_H

#include <QString>

// Endpoint của các daemon sensornode-core, override qua env khi chạy trên host.
namespace DaemonEndpoints {

inline QString telemetrySocket()
{
    return qEnvironmentVariable("SENSORNODE_TELEMETRY_SOCKET", QStringLiteral("/run/sensornode/telemetry.sock"));
}

inline QString otaSocket()
{
    return qEnvironmentVariable("SENSORNODE_OTA_SOCKET", QStringLiteral("/run/sensornode/ota.sock"));
}

inline QString settingsSocket()
{
    return qEnvironmentVariable("SENSORNODE_SETTINGS_SOCKET", QStringLiteral("/run/sensornode/settings.sock"));
}

inline QString displaySocket()
{
    return qEnvironmentVariable("SENSORNODE_DISPLAY_SOCKET", QStringLiteral("/run/sensornode/display.sock"));
}

inline QString telemetryDb()
{
    return qEnvironmentVariable("SENSORNODE_TELEMETRY_DB", QStringLiteral("/var/lib/sensornode/telemetry.db"));
}

inline QString timeSyncFlag()
{
    return qEnvironmentVariable("SENSORNODE_TIMESYNC_FLAG", QStringLiteral("/run/systemd/timesync/synchronized"));
}

inline QString dataMount()
{
    return qEnvironmentVariable("SENSORNODE_DATA_MOUNT", QStringLiteral("/data"));
}

inline QString touchCalibrationFile()
{
    return qEnvironmentVariable("TSLIB_CALIBFILE", QStringLiteral("/data/config/pointercal"));
}

inline QString journalctl()
{
    return qEnvironmentVariable("SENSORNODE_JOURNALCTL", QStringLiteral("journalctl"));
}

inline QString networkInterface()
{
    return qEnvironmentVariable("SENSORNODE_NET_INTERFACE", QStringLiteral("eth0"));
}

} // namespace DaemonEndpoints

#endif // DAEMON_ENDPOINTS_H
