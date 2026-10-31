#ifndef OTA_STATE_H
#define OTA_STATE_H

#include <QMetaType>

enum class OtaState {
    Idle,
    Checking,
    Available,
    Downloading,
    Flashing,
    Done,
    Failed
};

Q_DECLARE_METATYPE(OtaState)

#endif // OTA_STATE_H
