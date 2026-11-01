#include "app/AppWindow.h"

#include "model/HistoryModel.h"
#include "model/OtaModel.h"
#include "model/DisplayModel.h"
#include "model/SettingsModel.h"
#include "model/SystemStatusModel.h"
#include "model/TelemetryModel.h"
#include "screens/ChartScreen.h"
#include "screens/HomeScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/UpdatePopup.h"
#include "screens/UpdateProgressScreen.h"
#include "widgets/BottomNavBar.h"
#include "widgets/StatusHeader.h"

#include <QDebug>
#include <QStackedWidget>

AppWindow::AppWindow(QWidget *parent)
    : PageFrame(parent)
    , mTelemetry(new TelemetryModel(this))
    , mHistory(new HistoryModel(this))
    , mOta(new OtaModel(this))
    , mSettingsModel(new SettingsModel(this))
    , mDisplayModel(new DisplayModel(this))
    , mSystemStatus(new SystemStatusModel(this))
{
    buildUi();
    wireTelemetry();
    wireSettings();
    wireSystemStatus();
    wireOta();
    start();
}

AppWindow::~AppWindow() = default;

void AppWindow::buildUi()
{
    setWindowTitle("SensorNode");

    mHome = new HomeScreen(stack());
    mCharts = new ChartScreen(mHistory, stack());
    mSettings = new SettingsScreen(stack());
    mProgress = new UpdateProgressScreen(stack());

    addTab(BottomNavBar::Home, mHome);
    addTab(BottomNavBar::Dashboard, mCharts);
    addTab(BottomNavBar::Settings, mSettings);
    stack()->addWidget(mProgress);

    mPopup = new UpdatePopup(this);
    mPopup->hide();
}

void AppWindow::wireTelemetry()
{
    connect(mTelemetry, &TelemetryModel::sampleReceived, mHome, &HomeScreen::setValues);
    connect(mTelemetry, &TelemetryModel::sampleReceived, mCharts, &ChartScreen::reload);
    connect(mTelemetry, &TelemetryModel::macAddressChanged, mSettings, &SettingsScreen::setMacAddress);
}

void AppWindow::wireSettings()
{
    connect(mSettingsModel, &SettingsModel::brightnessChanged, mSettings, &SettingsScreen::setBrightness);
    connect(mSettingsModel, &SettingsModel::autoModeChanged, mSettings, &SettingsScreen::setOtaMode);
    connect(mSettingsModel, &SettingsModel::autoModeChanged, mOta, &OtaModel::setAutoMode);
    connect(mSettingsModel, &SettingsModel::availableChanged, mSettings, &SettingsScreen::setSettingsEnabled);
    connect(mSettings, &SettingsScreen::brightnessChangeRequested, mDisplayModel, &DisplayModel::setBrightness);
    connect(mSettings, &SettingsScreen::brightnessChangeRequested, mSettingsModel, &SettingsModel::setBrightness);
    connect(mSettings, &SettingsScreen::otaAutoModeChanged, mSettingsModel, &SettingsModel::setAutoMode);
}

void AppWindow::wireSystemStatus()
{
    connect(mSystemStatus, &SystemStatusModel::clockChanged, statusHeader(), &StatusHeader::setClock);
    connect(mSystemStatus, &SystemStatusModel::networkChanged, statusHeader(), &StatusHeader::setNetworkUp);
}

void AppWindow::wireOta()
{
    connect(mOta, &OtaModel::currentVersionChanged, mSettings, &SettingsScreen::setCurrentVersion);
    connect(mOta, &OtaModel::trialBootChanged, statusHeader(), &StatusHeader::setVerifying);
    connect(mOta, &OtaModel::trialBootChanged, mPopup, &UpdatePopup::setVerifying);
    connect(mSettings, &SettingsScreen::checkForUpdateRequested, this, &AppWindow::onCheckForUpdateRequested);
    connect(mOta, &OtaModel::upToDate, this, [this]() {
        mSettings->onCheckFinished("Up to date");
    });
    connect(mOta, &OtaModel::checkFailed, this, &AppWindow::onCheckFailed);

    connect(mOta, &OtaModel::updateAvailable, this, &AppWindow::onUpdateAvailable);
    connect(mOta, &OtaModel::stateChanged, this, &AppWindow::onOtaStateChanged);
    connect(mOta, &OtaModel::progressChanged, mProgress, &UpdateProgressScreen::setProgress);
    connect(mPopup, &UpdatePopup::updateConfirmed, mOta, &OtaModel::confirmUpdate);
    connect(mPopup, &UpdatePopup::cancelled, mOta, &OtaModel::dismissUpdate);

    connect(mProgress, &UpdateProgressScreen::cancelled, this, &AppWindow::onUpdateCancelled);
    connect(mProgress, &UpdateProgressScreen::rebootRequested, mOta, &OtaModel::reboot);
}

void AppWindow::start()
{
    showTab(BottomNavBar::Home);

    mTelemetry->start();
    mSettingsModel->start();
    mDisplayModel->start();
    mOta->start();
    mSystemStatus->start();
}

void AppWindow::onCheckForUpdateRequested()
{
    mSettings->onCheckStarted();
    mOta->checkForUpdate();
}

void AppWindow::onCheckFailed(const QString &reason)
{
    qWarning() << "OTA check failed:" << reason;
    mSettings->onCheckFinished("Check failed");
}

void AppWindow::onUpdateAvailable(const QString &current, const QString &available)
{
    mSettings->onCheckFinished("Check for updates");
    mPopup->setVersions(current, available);
}

void AppWindow::onOtaStateChanged(OtaState state)
{
    switch (state) {
    case OtaState::Available:
        showUpdatePopup();
        break;
    case OtaState::Downloading:
        mPopup->hide();
        mProgress->setTargetVersion(mOta->availableVersion());
        enterUpdateMode();
        break;
    case OtaState::Failed:
        mProgress->setFailureReason(mOta->lastError());
        enterUpdateMode();
        break;
    default:
        break;
    }
    mProgress->setState(state);
}

void AppWindow::onUpdateCancelled()
{
    mOta->dismissFailure();
    exitUpdateMode();
}

void AppWindow::showUpdatePopup()
{
    mPopup->setGeometry(rect());
    mPopup->raise();
    mPopup->show();
}

void AppWindow::enterUpdateMode()
{
    showTakeover(mProgress);
}

void AppWindow::exitUpdateMode()
{
    showTab(BottomNavBar::Settings);
}
