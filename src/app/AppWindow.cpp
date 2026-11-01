#include "app/AppWindow.h"

#include "app/DemoData.h"
#include "app/Stubs.h"
#include "model/HistoryModel.h"
#include "model/Locations.h"
#include "model/OtaModel.h"
#include "model/DisplayModel.h"
#include "model/SettingsModel.h"
#include "model/SystemStatusModel.h"
#include "model/TelemetryModel.h"
#include "screens/ChartScreen.h"
#include "screens/DisplayPage.h"
#include "screens/HomeScreen.h"
#include "screens/LanguageLocationPage.h"
#include "screens/LocationPage.h"
#include "screens/SamplingPage.h"
#include "screens/SettingsScreen.h"
#include "screens/UpdatePage.h"
#include "screens/UpdatePopup.h"
#include "screens/UpdateProgressScreen.h"
#include "widgets/BottomNavBar.h"
#include "widgets/StatusHeader.h"
#include "widgets/SubPage.h"

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

    mHome = new HomeScreen(mHistory, stack());
    mCharts = new ChartScreen(mHistory, stack());
    mSettings = new SettingsScreen(stack());
    mProgress = new UpdateProgressScreen(stack());

    addTab(BottomNavBar::Home, mHome);
    addTab(BottomNavBar::Dashboard, mCharts);
    addTab(BottomNavBar::Settings, mSettings);
    stack()->addWidget(mProgress);

    buildSettingsPages();

    mPopup = new UpdatePopup(this);
    mPopup->hide();
}

void AppWindow::buildSettingsPages()
{
    mDisplayPage = new DisplayPage(stack());
    mSamplingPage = new SamplingPage(stack());
    mLanguagePage = new LanguageLocationPage(stack());
    mLocationPage = new LocationPage(stack());
    mUpdatePage = new UpdatePage(stack());
    mServicePage = new SubPage(tr("Bảo trì"), stack());

    for (SubPage *page : {static_cast<SubPage *>(mDisplayPage), static_cast<SubPage *>(mSamplingPage),
                          static_cast<SubPage *>(mLanguagePage), static_cast<SubPage *>(mLocationPage),
                          static_cast<SubPage *>(mUpdatePage), mServicePage}) {
        addPage(page);
    }

    connect(mSettings, &SettingsScreen::displayRequested, this, [this]() { pushPage(mDisplayPage); });
    connect(mSettings, &SettingsScreen::samplingRequested, this, [this]() { pushPage(mSamplingPage); });
    connect(mSettings, &SettingsScreen::languageLocationRequested, this, [this]() { pushPage(mLanguagePage); });
    connect(mSettings, &SettingsScreen::updateRequested, this, [this]() { pushPage(mUpdatePage); });
    connect(mSettings, &SettingsScreen::serviceRequested, this, [this]() { pushPage(mServicePage); });
    connect(mLanguagePage, &LanguageLocationPage::locationRequested, this, [this]() { pushPage(mLocationPage); });

    mSettings->setTemperatureUnit(DemoData::temperatureUnit);
    mSettings->setSamplingPeriodText(SamplingPage::periodText(mSamplingPage->period()));
    mDisplayPage->setAutoDimMinutes(DemoData::autoDimMinutes);
    mLanguagePage->setLanguage(QLatin1String(DemoData::language));
    mLocationPage->setSelectedIndex(DemoData::locationIndex);
    showLocation(DemoData::locationIndex);
}

void AppWindow::wireTelemetry()
{
    connect(mTelemetry, &TelemetryModel::sampleReceived, mHome, &HomeScreen::setValues);
    connect(mTelemetry, &TelemetryModel::staleChanged, mHome, &HomeScreen::setStale);
    connect(mHome, &HomeScreen::metricRequested, this, [this](Metric metric) {
        mCharts->showMetric(metric);
        showTab(BottomNavBar::Dashboard);
    });
    connect(mTelemetry, &TelemetryModel::sampleReceived, mCharts, &ChartScreen::reload);
    connect(mTelemetry, &TelemetryModel::intervalChanged, this, &AppWindow::onTelemetryIntervalChanged);
}

void AppWindow::wireSettings()
{
    connect(mSettingsModel, &SettingsModel::brightnessChanged, mSettings, &SettingsScreen::setBrightness);
    connect(mSettingsModel, &SettingsModel::brightnessChanged, mDisplayPage, &DisplayPage::setBrightness);
    connect(mSettingsModel, &SettingsModel::autoModeChanged, mUpdatePage, &UpdatePage::setAutoNotify);
    connect(mSettingsModel, &SettingsModel::autoModeChanged, mOta, &OtaModel::setAutoMode);
    connect(mSettingsModel, &SettingsModel::availableChanged, mDisplayPage, &DisplayPage::setBrightnessEnabled);
    connect(mSettingsModel, &SettingsModel::availableChanged, mUpdatePage, &UpdatePage::setAutoNotifyEnabled);
    connect(mDisplayPage, &DisplayPage::brightnessPreviewed, mDisplayModel, &DisplayModel::setBrightness);
    connect(mDisplayPage, &DisplayPage::brightnessPreviewed, mSettings, &SettingsScreen::setBrightness);
    connect(mDisplayPage, &DisplayPage::brightnessCommitted, mSettingsModel, &SettingsModel::setBrightness);
    connect(mUpdatePage, &UpdatePage::autoNotifyRequested, mSettingsModel, &SettingsModel::setAutoMode);

    connect(mDisplayPage, &DisplayPage::autoDimRequested, this, [](int minutes) { Stubs::setAutoDim(minutes); });
    connect(mSamplingPage, &SamplingPage::periodRequested, this, &AppWindow::onSamplingPeriodRequested);
    connect(mSettings, &SettingsScreen::temperatureUnitRequested, this, &AppWindow::onTemperatureUnitRequested);
    connect(mLanguagePage, &LanguageLocationPage::languageRequested, this, &AppWindow::onLanguageRequested);
    connect(mLocationPage, &LocationPage::locationRequested, this, &AppWindow::onLocationRequested);
}

void AppWindow::wireSystemStatus()
{
    connect(mSystemStatus, &SystemStatusModel::clockChanged, statusHeader(), &StatusHeader::setClock);
    connect(mSystemStatus, &SystemStatusModel::networkChanged, statusHeader(), &StatusHeader::setNetworkUp);
    connect(mSystemStatus, &SystemStatusModel::clockChanged, mLocationPage, [this](const QString &text) {
        mLocationPage->setClockSynced(!text.isEmpty());
    });
}

void AppWindow::wireOta()
{
    connect(mOta, &OtaModel::currentVersionChanged, mSettings, &SettingsScreen::setCurrentVersion);
    connect(mOta, &OtaModel::currentVersionChanged, mUpdatePage, &UpdatePage::setCurrentVersion);
    connect(mOta, &OtaModel::trialBootChanged, mUpdatePage, &UpdatePage::setVerifying);
    connect(mOta, &OtaModel::trialBootChanged, statusHeader(), &StatusHeader::setVerifying);
    connect(mOta, &OtaModel::trialBootChanged, mPopup, &UpdatePopup::setVerifying);
    connect(mUpdatePage, &UpdatePage::checkRequested, this, &AppWindow::onCheckForUpdateRequested);
    connect(mOta, &OtaModel::upToDate, mUpdatePage, &UpdatePage::onUpToDate);
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
    mUpdatePage->onCheckStarted();
    mOta->checkForUpdate();
}

void AppWindow::onCheckFailed(const QString &reason)
{
    qWarning() << "OTA check failed:" << reason;
    mUpdatePage->onCheckFailed();
}

void AppWindow::onSamplingPeriodRequested(int seconds)
{
    mSamplingPage->beginApply(seconds);
    const bool ok = Stubs::setSamplingPeriod(seconds);
    mSamplingPage->finishApply(ok);
    mSettings->setSamplingPeriodText(SamplingPage::periodText(mSamplingPage->period()));
}

void AppWindow::onTelemetryIntervalChanged(int intervalMs)
{
    mSamplingPage->setPeriod(intervalMs / 1000);
    mSettings->setSamplingPeriodText(SamplingPage::periodText(mSamplingPage->period()));
}

void AppWindow::onTemperatureUnitRequested(TemperatureUnit unit)
{
    Stubs::setTemperatureUnit(unit);
    mHome->setTemperatureUnit(unit);
    mCharts->setTemperatureUnit(unit);
}

void AppWindow::onLanguageRequested(const QString &language)
{
    Stubs::setLanguage(language);
}

void AppWindow::onLocationRequested(int index)
{
    const int previous = mLocationPage->selectedIndex();
    const Location &location = Locations::all().at(index);

    mLocationPage->setSelectedIndex(index);
    const bool ok = Stubs::setLocation(location.nameEn, location.timeZone);
    mLocationPage->finishApply(previous, ok);

    if (ok) {
        showLocation(index);
        mSystemStatus->refresh();
        mLocationPage->refreshTimes();
    }
}

void AppWindow::showLocation(int index)
{
    const Location &location = Locations::all().at(index);
    Locations::apply(location);
    mLanguagePage->setLocationName(location.name);
}

void AppWindow::onUpdateAvailable(const QString &current, const QString &available)
{
    mUpdatePage->onUpdateFound();
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
