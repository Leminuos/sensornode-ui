#include "app/AppWindow.h"

#include "app/DemoData.h"
#include "app/Language.h"
#include "app/Stubs.h"
#include "model/DeviceInfoModel.h"
#include "model/ErrorLogModel.h"
#include "model/HistoryModel.h"
#include "model/Locations.h"
#include "model/NetworkModel.h"
#include "model/OtaModel.h"
#include "model/DisplayModel.h"
#include "ipc/DaemonEndpoints.h"
#include "model/SettingsModel.h"
#include "model/SystemStatusModel.h"
#include "model/TelemetryModel.h"
#include "model/TouchCalibration.h"
#include "screens/CalibrationIntroPage.h"
#include "screens/CalibrationPage.h"
#include "screens/ChartScreen.h"
#include "screens/DevicePage.h"
#include "screens/DisplayPage.h"
#include "screens/ErrorLogPage.h"
#include "screens/FirmwarePage.h"
#include "screens/HomeScreen.h"
#include "screens/LanguageLocationPage.h"
#include "screens/LocationPage.h"
#include "screens/SamplingPage.h"
#include "screens/SensorsPage.h"
#include "screens/ServicePage.h"
#include "screens/SettingsScreen.h"
#include "screens/UpdateFlowPage.h"
#include "screens/UpdatePage.h"
#include "widgets/BottomNavBar.h"
#include "widgets/BottomSheet.h"
#include "widgets/StatusHeader.h"
#include "widgets/SubPage.h"

#include <QDebug>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QVariant>

AppWindow::AppWindow(QWidget *parent)
    : PageFrame(parent)
    , mTelemetry(new TelemetryModel(this))
    , mHistory(new HistoryModel(this))
    , mOta(new OtaModel(this))
    , mSettingsModel(new SettingsModel(this))
    , mDisplayModel(new DisplayModel(this))
    , mSystemStatus(new SystemStatusModel(this))
    , mNetwork(new NetworkModel(this))
    , mDeviceInfo(new DeviceInfoModel(this))
    , mErrorLog(new ErrorLogModel(this))
{
    Language::instance().setCode(QLatin1String(DemoData::language));

    buildUi();
    wireTelemetry();
    wireSettings();
    wireSystemStatus();
    wireService();
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
    mUpdateFlow = new UpdateFlowPage(stack());

    addTab(BottomNavBar::Home, mHome);
    addTab(BottomNavBar::Dashboard, mCharts);
    addTab(BottomNavBar::Settings, mSettings);
    stack()->addWidget(mUpdateFlow);

    buildSettingsPages();
    buildServicePages();
    buildUpdateFlow();
    buildCalibration();

    Language::bind(this, [this]() { applyLanguageTexts(); });
}

void AppWindow::buildCalibration()
{
    mCalibrationIntro = new CalibrationIntroPage(stack());
    mCalibration = new CalibrationPage(stack());

    addPage(mCalibrationIntro);
    stack()->addWidget(mCalibration);

    connect(mServicePage, &ServicePage::calibrationRequested, this, [this]() { pushPage(mCalibrationIntro); });
    connect(mCalibrationIntro, &CalibrationIntroPage::startRequested, this, &AppWindow::startCalibration);

    connect(mCalibration, &CalibrationPage::pointsCaptured, this, &AppWindow::onCalibrationPoints);
    connect(mCalibration, &CalibrationPage::cancelled, this, &AppWindow::finishCalibration);
    connect(mCalibration, &CalibrationPage::kept, this, [this]() {
        Stubs::keepTouchCalibration();
        finishCalibration();
    });
    connect(mCalibration, &CalibrationPage::reverted, this, [this]() {
        Stubs::restoreTouchCalibration();
        finishCalibration();
    });
    connect(mCalibration, &CalibrationPage::restartRequested, this, [this]() {
        Stubs::restoreTouchCalibration();
        mCalibration->beginPoints();
    });
}

void AppWindow::applyLanguageTexts()
{
    mUpdateSheet->setTitle(tr("Có bản mới"));
    mUpdateSheet->setActions(tr("Để sau"), tr("Cập nhật"), RaisedButton::Primary);

    mRestartSheet->setTitle(tr("Khởi động lại thiết bị?"));
    mRestartSheet->setBody(tr("Việc đo sẽ dừng khoảng 1 phút."));
    mRestartSheet->setActions(tr("Huỷ"), tr("Khởi động lại"), RaisedButton::Danger);

    const QString period = SamplingPage::periodText(mSamplingPage->period());
    mSettings->setSamplingPeriodText(period);
    mSensorsPage->setPeriodText(period);
    mLanguagePage->setLocationName(Locations::displayName(Locations::all().at(mLocationPage->selectedIndex())));
}

void AppWindow::startCalibration()
{
    mCalibration->beginPoints();
    showFullScreenPage(mCalibration);
}

void AppWindow::finishCalibration()
{
    leaveOverlay();
}

void AppWindow::onCalibrationPoints(const QVector<QPointF> &touched)
{
    QVector<QPointF> targets;
    for (const QPoint &target : CalibrationPage::targets()) {
        targets.append(target);
    }

    AffineFit fit;
    if (!TouchCalibration::solve(touched, targets, &fit)) {
        mCalibration->beginPoints();
        return;
    }

    Pointercal current = TouchCalibration::identity(width(), height());
    QFile file(DaemonEndpoints::touchCalibrationFile());
    if (file.open(QFile::ReadOnly)) {
        TouchCalibration::parse(file.readAll(), &current);
    }

    Stubs::applyTouchCalibration(TouchCalibration::format(TouchCalibration::compose(fit, current)));
    mCalibration->beginVerify();
}

void AppWindow::buildUpdateFlow()
{
    mUpdateSheet = new BottomSheet(this);

    auto *versions = new QWidget(mUpdateSheet->panel());
    versions->setObjectName(QStringLiteral("versionRow"));
    versions->setAttribute(Qt::WA_StyledBackground);

    mSheetCurrent = new QLabel(versions);
    mSheetCurrent->setObjectName(QStringLiteral("versionCurrent"));
    auto *arrow = new QLabel(QStringLiteral("→"), versions);
    arrow->setObjectName(QStringLiteral("versionCurrent"));
    mSheetNext = new QLabel(versions);
    mSheetNext->setObjectName(QStringLiteral("versionNext"));

    auto *row = new QHBoxLayout(versions);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(10);
    row->addStretch();
    row->addWidget(mSheetCurrent);
    row->addWidget(arrow);
    row->addWidget(mSheetNext);
    row->addStretch();

    mUpdateSheet->setExtra(versions, 39);
}

void AppWindow::buildSettingsPages()
{
    mDisplayPage = new DisplayPage(stack());
    mSamplingPage = new SamplingPage(stack());
    mLanguagePage = new LanguageLocationPage(stack());
    mLocationPage = new LocationPage(stack());
    mUpdatePage = new UpdatePage(stack());
    mServicePage = new ServicePage(stack());

    for (SubPage *page : {static_cast<SubPage *>(mDisplayPage), static_cast<SubPage *>(mSamplingPage),
                          static_cast<SubPage *>(mLanguagePage), static_cast<SubPage *>(mLocationPage),
                          static_cast<SubPage *>(mUpdatePage), static_cast<SubPage *>(mServicePage)}) {
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

void AppWindow::buildServicePages()
{
    mFirmwarePage = new FirmwarePage(stack());
    mDevicePage = new DevicePage(stack());
    mSensorsPage = new SensorsPage(stack());
    mErrorLogPage = new ErrorLogPage(stack());

    for (SubPage *page : {static_cast<SubPage *>(mFirmwarePage), static_cast<SubPage *>(mDevicePage),
                          static_cast<SubPage *>(mSensorsPage), static_cast<SubPage *>(mErrorLogPage)}) {
        addPage(page);
    }

    connect(mServicePage, &ServicePage::firmwareRequested, this, [this]() { pushPage(mFirmwarePage); });
    connect(mServicePage, &ServicePage::deviceRequested, this, [this]() { pushPage(mDevicePage); });
    connect(mServicePage, &ServicePage::sensorsRequested, this, [this]() { pushPage(mSensorsPage); });
    connect(mServicePage, &ServicePage::errorLogRequested, this, [this]() { pushPage(mErrorLogPage); });

    mRestartSheet = new BottomSheet(this);

    mDevicePage->setMqttConnected(DemoData::mqttConnected);
    mSensorsPage->setPeriodText(SamplingPage::periodText(mSamplingPage->period()));
}

void AppWindow::wireService()
{
    connect(mFirmwarePage, &FirmwarePage::refreshRequested, mOta, &OtaModel::refreshStatus);
    connect(mFirmwarePage, &FirmwarePage::refreshRequested, mDeviceInfo, &DeviceInfoModel::refresh);
    connect(mDevicePage, &DevicePage::refreshRequested, mDeviceInfo, &DeviceInfoModel::refresh);
    connect(mErrorLogPage, &ErrorLogPage::refreshRequested, mErrorLog, &ErrorLogModel::reload);

    connect(mOta, &OtaModel::systemInfoChanged, mFirmwarePage, &FirmwarePage::setSystemInfo);
    connect(mDeviceInfo, &DeviceInfoModel::infoChanged, mFirmwarePage, &FirmwarePage::setDeviceInfo);
    connect(mDeviceInfo, &DeviceInfoModel::infoChanged, mDevicePage, &DevicePage::setDeviceInfo);
    connect(mTelemetry, &TelemetryModel::macAddressChanged, mDevicePage, &DevicePage::setMacAddress);
    connect(mTelemetry, &TelemetryModel::reportReceived, mSensorsPage, &SensorsPage::setReport);
    connect(mErrorLog, &ErrorLogModel::entriesChanged, mErrorLogPage, &ErrorLogPage::setEntries);
    connect(mErrorLog, &ErrorLogModel::failed, mErrorLogPage, &ErrorLogPage::showLoadFailed);

    connect(mFirmwarePage, &FirmwarePage::restartRequested, mRestartSheet, &BottomSheet::present);
    connect(mRestartSheet, &BottomSheet::accepted, this, []() { Stubs::restartDevice(); });
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
    connect(mSystemStatus, &SystemStatusModel::clockChanged, mLocationPage, [this](const QString &text) {
        mLocationPage->setClockSynced(!text.isEmpty());
    });
    connect(mNetwork, &NetworkModel::statusChanged, statusHeader(), &StatusHeader::setNetworkStatus);
}

void AppWindow::wireOta()
{
    connect(mOta, &OtaModel::currentVersionChanged, mSettings, &SettingsScreen::setCurrentVersion);
    connect(mOta, &OtaModel::currentVersionChanged, mUpdatePage, &UpdatePage::setCurrentVersion);
    connect(mOta, &OtaModel::trialBootChanged, mUpdatePage, &UpdatePage::setVerifying);
    connect(mOta, &OtaModel::trialBootChanged, statusHeader(), &StatusHeader::setVerifying);
    connect(mOta, &OtaModel::trialBootChanged, this, [this](bool trialBoot) {
        mUpdateSheet->setAcceptEnabled(!trialBoot);
    });
    connect(mUpdatePage, &UpdatePage::checkRequested, this, &AppWindow::onCheckForUpdateRequested);
    connect(mOta, &OtaModel::upToDate, mUpdatePage, &UpdatePage::onUpToDate);
    connect(mOta, &OtaModel::checkFailed, this, &AppWindow::onCheckFailed);

    connect(mOta, &OtaModel::updateAvailable, this, &AppWindow::onUpdateAvailable);
    connect(mOta, &OtaModel::stateChanged, this, &AppWindow::onOtaStateChanged);
    connect(mOta, &OtaModel::progressChanged, mUpdateFlow, &UpdateFlowPage::setProgress);
    connect(mUpdateSheet, &BottomSheet::accepted, mOta, &OtaModel::confirmUpdate);
    connect(mUpdateSheet, &BottomSheet::rejected, mOta, &OtaModel::dismissUpdate);

    connect(mUpdateFlow, &UpdateFlowPage::closeRequested, this, &AppWindow::onUpdateClosed);
    connect(mUpdateFlow, &UpdateFlowPage::retryRequested, mOta, &OtaModel::confirmUpdate);
    connect(mUpdateFlow, &UpdateFlowPage::rebootRequested, mOta, &OtaModel::reboot);
    connect(mUpdateFlow, &UpdateFlowPage::rollbackAcknowledged, this, [this]() {
        showTab(BottomNavBar::Home);
    });
}

void AppWindow::start()
{
    showTab(BottomNavBar::Home);

    mTelemetry->start();
    mSettingsModel->start();
    mDisplayModel->start();
    mOta->start();
    mSystemStatus->start();
    mNetwork->start();

    checkRollback();

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
    const QString period = SamplingPage::periodText(mSamplingPage->period());
    mSettings->setSamplingPeriodText(period);
    mSensorsPage->setPeriodText(period);
}

void AppWindow::onTelemetryIntervalChanged(int intervalMs)
{
    mSamplingPage->setPeriod(intervalMs / 1000);
    const QString period = SamplingPage::periodText(mSamplingPage->period());
    mSettings->setSamplingPeriodText(period);
    mSensorsPage->setPeriodText(period);
}

void AppWindow::onTemperatureUnitRequested(TemperatureUnit unit)
{
    Stubs::setTemperatureUnit(unit);
    mHome->setTemperatureUnit(unit);
    mCharts->setTemperatureUnit(unit);
    mSensorsPage->setTemperatureUnit(unit);
}

void AppWindow::onLanguageRequested(const QString &language)
{
    Stubs::setLanguage(language);
    Language::instance().setCode(language);
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
    mLanguagePage->setLocationName(Locations::displayName(location));
}

void AppWindow::onUpdateAvailable(const QString &current, const QString &available)
{
    mUpdatePage->onUpdateFound();
    mSheetCurrent->setText(current);
    mSheetNext->setText(available);
}

void AppWindow::onOtaStateChanged(OtaState state)
{
    if (state != OtaState::Available) {
        mUpdateSheet->dismiss();
    }

    if (!mOta->availableVersion().isEmpty()) {
        mUpdateVersion = mOta->availableVersion();
    }

    switch (state) {
    case OtaState::Idle:
    case OtaState::Checking:
        if (mode() == Mode::Takeover) {
            leaveOverlay();
        }
        return;
    case OtaState::Available:
        showUpdateSheet();
        return;
    case OtaState::Downloading:
        mUpdateFlow->showDownloading(mUpdateVersion, sizeMb());
        break;
    case OtaState::Flashing:
        mUpdateFlow->showInstalling();
        break;
    case OtaState::Done:
        mUpdateFlow->showInstalled(mUpdateVersion, mOta->rebootFailed());
        break;
    case OtaState::Rebooting:
        mUpdateFlow->showRebooting();
        break;
    case OtaState::Failed:
        if (mOta->failedPhase() == OtaState::Flashing) {
            mUpdateFlow->showInstallFailed(mOta->currentVersion());
        } else {
            mUpdateFlow->showDownloadFailed();
        }
        break;
    }

    showTakeover(mUpdateFlow);
}

void AppWindow::onUpdateClosed()
{
    mOta->dismissFailure();
}

void AppWindow::showUpdateSheet()
{
    const int size = sizeMb();
    mUpdateSheet->setBody(size > 0 ? tr("Khoảng %1 MB. Thiết bị khởi động lại sau khi cài.").arg(size)
                                   : tr("Thiết bị khởi động lại sau khi cài."));
    mUpdateSheet->present();
}

int AppWindow::sizeMb() const
{
    const qint64 bytes = mOta->availableSize();
    return bytes > 0 ? static_cast<int>(qMax<qint64>(1, bytes / (1000 * 1000))) : 0;
}

void AppWindow::checkRollback()
{
    const QString failed = Stubs::rolledBackVersion();
    if (!failed.isEmpty()) {
        mUpdateFlow->showRolledBack(mOta->currentVersion(), failed);
        showTakeover(mUpdateFlow);
    }
}
