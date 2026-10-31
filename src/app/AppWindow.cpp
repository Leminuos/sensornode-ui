#include "app/AppWindow.h"

#include "model/HistoryModel.h"
#include "model/OtaModel.h"
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
#include <QVBoxLayout>

AppWindow::AppWindow(QWidget *parent)
    : QWidget(parent)
    , mTelemetry(new TelemetryModel(this))
    , mHistory(new HistoryModel(this))
    , mOta(new OtaModel(this))
{
    buildUi();
    wireNavigation();
    wireTelemetry();
    wireOta();
    start();
}

AppWindow::~AppWindow() = default;

void AppWindow::buildUi()
{
    setFixedSize(240, 320);
    setWindowTitle("SensorNode");
    setObjectName("AppRoot");

    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    mStatusHeader = new StatusHeader(this);
    rootLayout->addWidget(mStatusHeader);

    mStack = new QStackedWidget(this);
    rootLayout->addWidget(mStack, 1);

    mNavBar = new BottomNavBar(this);
    rootLayout->addWidget(mNavBar);

    mHome = new HomeScreen(mStack);
    mCharts = new ChartScreen(mHistory, mStack);
    mSettings = new SettingsScreen(mStack);
    mProgress = new UpdateProgressScreen(mStack);

    mStack->addWidget(mHome);
    mStack->addWidget(mCharts);
    mStack->addWidget(mSettings);
    mStack->addWidget(mProgress);
    mPopup = new UpdatePopup(this);
    mPopup->hide();
}

void AppWindow::wireNavigation()
{
    connect(mNavBar, &BottomNavBar::navRequested, this, &AppWindow::onNavRequested);
}

void AppWindow::wireTelemetry()
{
    connect(mTelemetry, &TelemetryModel::onlineChanged, mStatusHeader, &StatusHeader::setOnlineStatus);
    connect(mTelemetry, &TelemetryModel::sampleReceived, mHome, &HomeScreen::setValues);
    connect(mTelemetry, &TelemetryModel::sampleReceived, mCharts, &ChartScreen::reload);
}

void AppWindow::wireOta()
{
    connect(mOta, &OtaModel::currentVersionChanged, mSettings, &SettingsScreen::setCurrentVersion);
    connect(mOta, &OtaModel::autoModeChanged, mSettings, &SettingsScreen::setOtaMode);
    connect(mOta, &OtaModel::trialBootChanged, mStatusHeader, &StatusHeader::setVerifying);
    connect(mOta, &OtaModel::trialBootChanged, mPopup, &UpdatePopup::setVerifying);
    connect(mSettings, &SettingsScreen::otaAutoModeChanged, mOta, &OtaModel::setAutoMode);
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
    mStack->setCurrentWidget(mHome);
    mNavBar->setActive(BottomNavBar::Home);

    mTelemetry->start();
    mOta->start();
}

void AppWindow::onNavRequested(int tab)
{
    switch (tab) {
    case BottomNavBar::Home:      mStack->setCurrentWidget(mHome);     break;
    case BottomNavBar::Dashboard: mStack->setCurrentWidget(mCharts);   break;
    case BottomNavBar::Settings:  mStack->setCurrentWidget(mSettings); break;
    default: break;
    }
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
    mStack->setCurrentWidget(mProgress);
    mNavBar->hide();
}

void AppWindow::exitUpdateMode()
{
    mStack->setCurrentWidget(mSettings);
    mNavBar->setActive(BottomNavBar::Settings);
    mNavBar->show();
}
