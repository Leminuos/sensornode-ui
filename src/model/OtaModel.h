#ifndef OTA_MODEL_H
#define OTA_MODEL_H

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTimer>

#include "ipc/DaemonClient.h"
#include "model/OtaState.h"

class OtaModel : public QObject
{
    Q_OBJECT

public:
    explicit OtaModel(QObject *parent = nullptr, const QString &socketPath = QString());

    void start();

    QString currentVersion() const { return mCurrentVersion; }
    QString availableVersion() const { return mAvailableVersion; }
    OtaState state() const { return mState; }
    QString lastError() const { return mLastError; }
    OtaState failedPhase() const { return mFailedPhase; }
    bool rebootFailed() const { return mRebootFailed; }
    qint64 availableSize() const { return mAvailableSize; }
    bool trialBoot() const { return mTrialBoot; }
    QString rolledBackVersion() const { return mRolledBack; }

public slots:
    void setAutoMode(bool autoMode);
    void checkForUpdate();
    void refreshStatus();
    void confirmUpdate();
    void dismissUpdate();
    void dismissFailure();
    void reboot();
    void dismissRollback();

signals:
    void currentVersionChanged(const QString &version);
    void trialBootChanged(bool trialBoot);
    // Version vừa bị U-Boot rollback, rỗng khi đã dismiss.
    void rolledBackChanged(const QString &version);
    void systemInfoChanged(const OtaSystemInfo &info);

    void updateAvailable(const QString &current, const QString &available);
    void upToDate();
    void checkFailed(const QString &reason);

    void stateChanged(OtaState state);
    void progressChanged(int percent);

private:
    void onConnected();
    void onDisconnected();
    void onMessage(const QJsonObject &message);
    void onReply(const QJsonObject &reply);
    void applyOta(const QJsonObject &ota, bool live);

    void setState(OtaState state);
    void setTrialBoot(bool trialBoot);
    void fail(const QString &reason);
    bool isInstalling() const;
    bool isBusy() const;
    void finishCheck();
    void announceUpdate();

    DaemonClient mClient;
    bool mAutoMode = false;

    OtaState mState = OtaState::Idle;
    QString mCurrentVersion;
    QString mAvailableVersion;
    QString mLastError;
    OtaState mFailedPhase = OtaState::Downloading;
    bool mRebootFailed = false;
    qint64 mAvailableSize = -1;
    int mCheckId = -1;
    bool mTrialBoot = false;
    QString mRolledBack;
    QTimer mTrialPoll;
};

#endif // OTA_MODEL_H
