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

inline QString telemetryDb()
{
    return qEnvironmentVariable("SENSORNODE_TELEMETRY_DB", QStringLiteral("/var/lib/sensornode/telemetry.db"));
}

} // namespace DaemonEndpoints

#endif // DAEMON_ENDPOINTS_H
