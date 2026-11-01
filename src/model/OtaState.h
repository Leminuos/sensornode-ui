#ifndef OTA_STATE_H
#define OTA_STATE_H

#include <QMetaType>
#include <QString>

enum class OtaState {
    Idle,
    Checking,
    Available,
    Downloading,
    Flashing,
    Done,
    Failed
};

struct OtaSystemInfo
{
    QString version;
    QString runningSlot;
    QString activeSlot;
    bool trialBoot = false;
    int bootCount = -1;
    int bootLimit = -1;
};

Q_DECLARE_METATYPE(OtaState)
Q_DECLARE_METATYPE(OtaSystemInfo)

#endif // OTA_STATE_H
