#include "model/OtaModel.h"

#include "ipc/DaemonEndpoints.h"

#include <QDebug>

static const QString kDaemonDown = QStringLiteral("OTA daemon not running");
static const QString kTrialBootError = QStringLiteral("trial boot");
static constexpr int kTrialPollMs = 3000;

static bool isTrialBoot(const QJsonObject &system)
{
    const QString running = system.value(QStringLiteral("running_slot")).toString();
    return system.value(QStringLiteral("ustate")).toInt() == 1 && !running.isEmpty()
        && running == system.value(QStringLiteral("active_slot")).toString();
}

OtaModel::OtaModel(QObject *parent, const QString &socketPath)
    : QObject(parent)
    , mClient(socketPath.isEmpty() ? DaemonEndpoints::otaSocket() : socketPath)
{
    connect(&mClient, &DaemonClient::connected, this, &OtaModel::onConnected);
    connect(&mClient, &DaemonClient::disconnected, this, &OtaModel::onDisconnected);
    connect(&mClient, &DaemonClient::messageReceived, this, &OtaModel::onMessage);

    // Sau khi boot vào bản mới, ota-confirm-boot cần một lúc để kiểm tra rồi commit.
    // Trong lúc đó trialBoot() là true và otad từ chối install. otad không phát event
    // khi commit nên model hỏi lại status định kỳ tới khi hết boot thử.
    mTrialPoll.setInterval(kTrialPollMs);
    connect(&mTrialPoll, &QTimer::timeout, this, [this]() {
        mClient.sendCommand(QStringLiteral("status"));
    });
}

void OtaModel::start()
{
    mClient.start();
}

void OtaModel::onConnected()
{
    mClient.sendCommand(QStringLiteral("status"));
}

void OtaModel::onDisconnected()
{
    if (isInstalling()) {
        fail(QStringLiteral("Lost connection to OTA daemon"));
    }

    if (mState == OtaState::Checking) {
        emit checkFailed(kDaemonDown);
        finishCheck();
    }

    mCheckId = -1;
    setTrialBoot(false);
}

void OtaModel::onMessage(const QJsonObject &message)
{
    const QString type = message.value(QStringLiteral("type")).toString();
    const QJsonObject ota = message.value(QStringLiteral("ota")).toObject();

    if (type == QLatin1String("state") || type == QLatin1String("progress") || type == QLatin1String("available")) {
        applyOta(ota, /*live=*/true);
    } else if (type == QLatin1String("reply")) {
        onReply(message);
    }
}

void OtaModel::onReply(const QJsonObject &reply)
{
    const QString cmd = reply.value(QStringLiteral("cmd")).toString();
    const bool ok = reply.value(QStringLiteral("ok")).toBool();
    const QString error = reply.value(QStringLiteral("error")).toString();
    const bool userCheck = reply.value(QStringLiteral("id")).toInt(-2) == mCheckId;

    if (cmd == QLatin1String("status")) {
        if (userCheck) mCheckId = -1;

        if (!ok) {
            if (userCheck) {
                emit checkFailed(error);
                finishCheck();
            }
            return;
        }

        const QJsonObject system = reply.value(QStringLiteral("system")).toObject();
        setTrialBoot(isTrialBoot(system));

        const QString version = system.value(QStringLiteral("sw_version")).toString();

        if (!version.isEmpty() && version != mCurrentVersion) {
            mCurrentVersion = version;
            emit currentVersionChanged(mCurrentVersion);
        }

        applyOta(reply.value(QStringLiteral("ota")).toObject(), /*live=*/false);

        if (userCheck && !isBusy()) {
            if (!mAvailableVersion.isEmpty()) {
                announceUpdate();
            } else {
                emit upToDate();
                finishCheck();
            }
        }
    } else if (cmd == QLatin1String("install") && !ok) {
        if (error == kTrialBootError) {
            setTrialBoot(true);
            dismissUpdate();
        } else {
            fail(error);
        }
    } else if (cmd == QLatin1String("reboot") && !ok) {
        qWarning() << "OTA reboot rejected:" << error;
        emit stateChanged(mState);
    }
}

void OtaModel::applyOta(const QJsonObject &ota, bool live)
{
    const QString daemonState = ota.value(QStringLiteral("state")).toString();
    if (daemonState == QLatin1String("downloading")) {
        setState(OtaState::Downloading);
    } else if (daemonState == QLatin1String("installing")) {
        setState(OtaState::Flashing);
    } else if (daemonState == QLatin1String("done") || daemonState == QLatin1String("rebooting")) {
        const bool rebootFailed = mState == OtaState::Done && !ota.value(QStringLiteral("error")).toString().isEmpty();
        setState(OtaState::Done);
        if (live && rebootFailed) {
            emit stateChanged(mState);
        }
    } else if (daemonState == QLatin1String("failed")) {
        if (live || isInstalling()) {
            fail(ota.value(QStringLiteral("error")).toString());
        }
    } else if (daemonState == QLatin1String("idle") && isInstalling()) {
        fail(QStringLiteral("Update interrupted"));
    }

    const QString available = ota.value(QStringLiteral("available")).toObject().value(QStringLiteral("version")).toString();
    if (available != mAvailableVersion) {
        mAvailableVersion = available;
        if (mAvailableVersion.isEmpty()) {
            dismissUpdate();
        } else if (mAutoMode && !isBusy()) {
            announceUpdate();
        }
    }

    const QJsonValue percent = ota.value(QStringLiteral("percent"));
    if (isInstalling() && percent.isDouble()) {
        // 100% của otad chỉ là SWUpdate đọc hết image, còn flush xuống eMMC và ghi bootenv mới tới done.
        const int shown = mState == OtaState::Flashing ? qMin(percent.toInt(), 99) : percent.toInt();
        emit progressChanged(shown);
    }
}

void OtaModel::setAutoMode(bool autoMode)
{
    if (autoMode == mAutoMode) return;
    mAutoMode = autoMode;

    if (mAutoMode && !isBusy() && !mAvailableVersion.isEmpty() && mState != OtaState::Available) {
        announceUpdate();
    }
}

void OtaModel::checkForUpdate()
{
    if (isBusy() || mCheckId >= 0) return;

    mCheckId = mClient.sendCommand(QStringLiteral("status"));
    if (mCheckId < 0) {
        emit checkFailed(kDaemonDown);
        return;
    }

    if (mState == OtaState::Idle) {
        setState(OtaState::Checking);
    }
}

void OtaModel::confirmUpdate()
{
    if (isInstalling() || mTrialBoot) return;
    
    if (mClient.sendCommand(QStringLiteral("install")) < 0) {
        fail(kDaemonDown);
    }
}

void OtaModel::dismissUpdate()
{
    if (mState == OtaState::Available) {
        setState(OtaState::Idle);
    }
}

void OtaModel::dismissFailure()
{
    if (mState == OtaState::Failed) {
        setState(OtaState::Idle);
    }
}

void OtaModel::reboot()
{
    if (mClient.sendCommand(QStringLiteral("reboot")) < 0) {
        qWarning() << "OTA reboot:" << kDaemonDown;
        emit stateChanged(mState);
    }
}

void OtaModel::setState(OtaState state)
{
    if (mState == state) return;
    
    mState = state;
    emit stateChanged(mState);
}

void OtaModel::setTrialBoot(bool trialBoot)
{
    if (mTrialBoot == trialBoot) return;

    mTrialBoot = trialBoot;
    if (mTrialBoot) mTrialPoll.start();
    else mTrialPoll.stop();

    emit trialBootChanged(mTrialBoot);
}

void OtaModel::fail(const QString &reason)
{
    mLastError = reason.isEmpty() ? QStringLiteral("Update failed") : reason;
    setState(OtaState::Failed);
}

bool OtaModel::isInstalling() const
{
    return mState == OtaState::Downloading || mState == OtaState::Flashing;
}

bool OtaModel::isBusy() const
{
    return isInstalling() || mState == OtaState::Done || mState == OtaState::Failed;
}

void OtaModel::finishCheck()
{
    if (mState == OtaState::Checking) {
        setState(OtaState::Idle);
    }
}

void OtaModel::announceUpdate()
{
    emit updateAvailable(mCurrentVersion, mAvailableVersion);
    setState(OtaState::Available);
}
