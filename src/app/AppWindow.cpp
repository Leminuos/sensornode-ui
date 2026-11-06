#include "app/AppWindow.h"

#include "app/AutoDimmer.h"
#include "app/DemoData.h"
#include "app/Language.h"
#include "app/Stubs.h"
#include "model/DeviceInfoModel.h"
#include "model/ErrorLogModel.h"
#include "model/HistoryModel.h"
#include "model/Locations.h"
#include "model/NetworkModel.h"
#include "model/OtaModel.h"
#include "model/PowerModel.h"
#include "model/DisplayModel.h"
#include "ipc/DaemonEndpoints.h"
#include "model/SettingsModel.h"
#include "model/SystemStatusModel.h"
#include "model/TelemetryModel.h"
#include "model/TouchCalibration.h"
#include "screens/AdvancedPage.h"
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
#include "screens/NetworkDetailPage.h"
#include "screens/NetworkPage.h"
#include "screens/SamplingPage.h"
#include "screens/SensorsPage.h"
#include "screens/ServicePage.h"
#include "screens/SettingsScreen.h"
#include "screens/SystemPage.h"
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
    , mAutoDimmer(new AutoDimmer(this))
    , mPower(new PowerModel(this))
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

    connect(mAdvancedPage, &AdvancedPage::calibrationRequested, this, [this]() { pushPage(mCalibrationIntro); });
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

    mFactoryResetSheet->setTitle(tr("Khôi phục cài đặt gốc?"));
    mFactoryResetSheet->setBody(tr("Mọi cài đặt sẽ về mặc định. Dữ liệu đo được giữ nguyên."));
    mFactoryResetSheet->setActions(tr("Huỷ"), tr("Khôi phục"), RaisedButton::Danger);

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
    mAutoDimmer->setTimeoutMinutes(DemoData::autoDimMinutes);
    mLanguagePage->setLanguage(QLatin1String(DemoData::language));
    mLocationPage->setSelectedIndex(DemoData::locationIndex);
    showLocation(DemoData::locationIndex);
}

void AppWindow::buildServicePages()
{
    mDevicePage = new DevicePage(stack());
    mSystemPage = new SystemPage(stack());
    mNetworkPage = new NetworkPage(stack());
    mEthernetPage = new NetworkDetailPage(NetworkDetailPage::Ethernet, stack());
    mWifiPage = new NetworkDetailPage(NetworkDetailPage::Wifi, stack());
    mAdvancedPage = new AdvancedPage(stack());
    mFirmwarePage = new FirmwarePage(stack());
    mSensorsPage = new SensorsPage(stack());
    mErrorLogPage = new ErrorLogPage(stack());

    for (SubPage *page : {static_cast<SubPage *>(mDevicePage), static_cast<SubPage *>(mSystemPage),
                          static_cast<SubPage *>(mNetworkPage), static_cast<SubPage *>(mEthernetPage),
                          static_cast<SubPage *>(mWifiPage), static_cast<SubPage *>(mAdvancedPage),
                          static_cast<SubPage *>(mFirmwarePage), static_cast<SubPage *>(mSensorsPage),
                          static_cast<SubPage *>(mErrorLogPage)}) {
        addPage(page);
    }

    connect(mServicePage, &ServicePage::deviceRequested, this, [this]() { pushPage(mDevicePage); });
    connect(mServicePage, &ServicePage::systemRequested, this, [this]() { pushPage(mSystemPage); });
    connect(mServicePage, &ServicePage::networkRequested, this, [this]() { pushPage(mNetworkPage); });
    connect(mServicePage, &ServicePage::advancedRequested, this, [this]() { pushPage(mAdvancedPage); });
    connect(mNetworkPage, &NetworkPage::ethernetRequested, this, [this]() { pushPage(mEthernetPage); });
    connect(mNetworkPage, &NetworkPage::wifiRequested, this, [this]() { pushPage(mWifiPage); });
    connect(mAdvancedPage, &AdvancedPage::firmwareRequested, this, [this]() { pushPage(mFirmwarePage); });
    connect(mAdvancedPage, &AdvancedPage::sensorsRequested, this, [this]() { pushPage(mSensorsPage); });
    connect(mAdvancedPage, &AdvancedPage::errorLogRequested, this, [this]() { pushPage(mErrorLogPage); });

    mRestartSheet = new BottomSheet(this);
    mFactoryResetSheet = new BottomSheet(this);

    mSensorsPage->setPeriodText(SamplingPage::periodText(mSamplingPage->period()));
}

void AppWindow::wireService()
{
    connect(mFirmwarePage, &FirmwarePage::refreshRequested, mOta, &OtaModel::refreshStatus);
    connect(mFirmwarePage, &FirmwarePage::refreshRequested, mDeviceInfo, &DeviceInfoModel::refresh);
    connect(mDevicePage, &DevicePage::refreshRequested, mDeviceInfo, &DeviceInfoModel::refresh);
    connect(mSystemPage, &SystemPage::refreshRequested, mDeviceInfo, &DeviceInfoModel::refresh);
    connect(mErrorLogPage, &ErrorLogPage::refreshRequested, mErrorLog, &ErrorLogModel::reload);

    connect(mOta, &OtaModel::systemInfoChanged, mFirmwarePage, &FirmwarePage::setSystemInfo);
    connect(mOta, &OtaModel::currentVersionChanged, mSystemPage, &SystemPage::setFirmwareVersion);
    connect(mDeviceInfo, &DeviceInfoModel::infoChanged, mFirmwarePage, &FirmwarePage::setDeviceInfo);
    connect(mDeviceInfo, &DeviceInfoModel::infoChanged, mDevicePage, &DevicePage::setDeviceInfo);
    connect(mDeviceInfo, &DeviceInfoModel::infoChanged, mSystemPage, &SystemPage::setDeviceInfo);
    connect(mTelemetry, &TelemetryModel::nodeIdChanged, mDevicePage, &DevicePage::setNodeId);
    connect(mTelemetry, &TelemetryModel::macAddressChanged, mDevicePage, &DevicePage::setMacAddress);
    connect(mTelemetry, &TelemetryModel::reportReceived, mSensorsPage, &SensorsPage::setReport);
    connect(mTelemetry, &TelemetryModel::mqttConnectedChanged, mNetworkPage, &NetworkPage::setServerConnected);
    connect(mErrorLog, &ErrorLogModel::entriesChanged, mErrorLogPage, &ErrorLogPage::setEntries);
    connect(mErrorLog, &ErrorLogModel::failed, mErrorLogPage, &ErrorLogPage::showLoadFailed);

    connect(mSystemPage, &SystemPage::restartRequested, this, [this]() {
        mRestartSheet->setBody(tr("Việc đo sẽ dừng khoảng 1 phút."));
        mRestartSheet->present();
    });
    connect(mRestartSheet, &BottomSheet::accepted, mPower, &PowerModel::reboot);
    connect(mPower, &PowerModel::rebootFailed, this, [this]() {
        mRestartSheet->setBody(tr("Không khởi động lại được. Hãy thử lại."));
        mRestartSheet->present();
    });
    connect(mServicePage, &ServicePage::factoryResetRequested, this, &AppWindow::showFactoryResetSheet);
    connect(mFactoryResetSheet, &BottomSheet::accepted, this, &AppWindow::onFactoryResetAccepted);
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
    connect(mTelemetry, &TelemetryModel::intervalApplied, this, &AppWindow::onIntervalApplied);
}

void AppWindow::wireSettings()
{
    connect(mSettingsModel, &SettingsModel::brightnessChanged, mSettings, &SettingsScreen::setBrightness);
    connect(mSettingsModel, &SettingsModel::brightnessChanged, mDisplayPage, &DisplayPage::setBrightness);
    connect(mSettingsModel, &SettingsModel::autoModeChanged, mUpdatePage, &UpdatePage::setAutoNotify);
    connect(mSettingsModel, &SettingsModel::autoModeChanged, mOta, &OtaModel::setAutoMode);
    connect(mSettingsModel, &SettingsModel::availableChanged, mDisplayPage, &DisplayPage::setBrightnessEnabled);
    connect(mSettingsModel, &SettingsModel::availableChanged, mUpdatePage, &UpdatePage::setAutoNotifyEnabled);
    connect(mSettingsModel, &SettingsModel::brightnessChanged, mDisplayModel, &DisplayModel::setBrightness);
    connect(mDisplayPage, &DisplayPage::brightnessPreviewed, mDisplayModel, &DisplayModel::setBrightness);
    connect(mDisplayPage, &DisplayPage::brightnessPreviewed, mSettings, &SettingsScreen::setBrightness);
    connect(mDisplayPage, &DisplayPage::brightnessCommitted, mSettingsModel, &SettingsModel::setBrightness);
    connect(mUpdatePage, &UpdatePage::autoNotifyRequested, mSettingsModel, &SettingsModel::setAutoMode);

    connect(mSettingsModel, &SettingsModel::autoDimChanged, mDisplayPage, &DisplayPage::setAutoDimMinutes);
    connect(mSettingsModel, &SettingsModel::autoDimChanged, mAutoDimmer, &AutoDimmer::setTimeoutMinutes);
    connect(mSettingsModel, &SettingsModel::temperatureUnitChanged, this, &AppWindow::applyTemperatureUnit);
    connect(mSettingsModel, &SettingsModel::languageChanged, this, &AppWindow::applyLanguage);
    connect(mSettingsModel, &SettingsModel::locationChanged, this, [this](const QString &id) {
        const int index = Locations::indexOf(id);
        if (index >= 0) applyLocation(index);
    });
    connect(mSettingsModel, &SettingsModel::resetFinished, this, &AppWindow::onResetFinished);

    connect(mDisplayPage, &DisplayPage::autoDimRequested, mSettingsModel, &SettingsModel::setAutoDim);
    connect(mDisplayPage, &DisplayPage::autoDimRequested, mAutoDimmer, &AutoDimmer::setTimeoutMinutes);

    connect(mAutoDimmer, &AutoDimmer::offChanged, mDisplayModel, &DisplayModel::setOff);
    connect(stack(), &QStackedWidget::currentChanged, this, [this]() {
        const QWidget *page = currentPage();
        mAutoDimmer->setInhibited(page == mUpdateFlow || page == mCalibration);
    });
    connect(mSamplingPage, &SamplingPage::periodRequested, this, &AppWindow::onSamplingPeriodRequested);
    connect(mSettings, &SettingsScreen::temperatureUnitRequested, this, &AppWindow::onTemperatureUnitRequested);
    connect(mLanguagePage, &LanguageLocationPage::languageRequested, this, &AppWindow::onLanguageRequested);
    connect(mLocationPage, &LocationPage::locationRequested, this, &AppWindow::onLocationRequested);
}

void AppWindow::wireSystemStatus()
{
    connect(mSystemStatus, &SystemStatusModel::clockChanged, statusHeader(), &StatusHeader::setClock);
    connect(mSystemStatus, &SystemStatusModel::clockChanged, this, [this](const QString &text) {
        mLocationPage->setClockSynced(!text.isEmpty());
        mNetworkPage->setTimeSynced(!text.isEmpty());
    });

    connect(mNetwork, &NetworkModel::statusChanged, statusHeader(), &StatusHeader::setNetworkStatus);
    connect(mNetwork, &NetworkModel::statusChanged, mNetworkPage, &NetworkPage::setStatus);
    connect(mNetwork, &NetworkModel::statusChanged, mEthernetPage, &NetworkDetailPage::setStatus);
    connect(mNetwork, &NetworkModel::statusChanged, mWifiPage, &NetworkDetailPage::setStatus);
    connect(mNetworkPage, &NetworkPage::refreshRequested, mNetwork, &NetworkModel::refreshSsid);
    connect(mWifiPage, &NetworkDetailPage::refreshRequested, mNetwork, &NetworkModel::refreshSsid);
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
    connect(mOta, &OtaModel::rolledBackChanged, this, &AppWindow::onRolledBack);
    connect(mUpdateFlow, &UpdateFlowPage::rollbackAcknowledged, this, [this]() {
        mOta->dismissRollback();
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
    mPower->start();
    mSystemStatus->start();
    mNetwork->start();
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
    if (!mTelemetry->setInterval(seconds * 1000)) {
        onIntervalApplied(false);
    }
}

void AppWindow::onIntervalApplied(bool ok)
{
    const int seconds = mSamplingPage->pendingPeriod();
    mSamplingPage->finishApply(ok);
    if (ok) {
        mSettingsModel->setSamplingPeriod(seconds);
    }
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
    mSettingsModel->setTemperatureUnit(unit);
    applyTemperatureUnit(unit);
}

void AppWindow::applyTemperatureUnit(TemperatureUnit unit)
{
    mSettings->setTemperatureUnit(unit);
    mHome->setTemperatureUnit(unit);
    mCharts->setTemperatureUnit(unit);
    mSensorsPage->setTemperatureUnit(unit);
}

void AppWindow::onLanguageRequested(const QString &language)
{
    mSettingsModel->setLanguage(language);
    Language::instance().setCode(language);
}

void AppWindow::applyLanguage(const QString &language)
{
    mLanguagePage->setLanguage(language);
    Language::instance().setCode(language);
}

void AppWindow::onLocationRequested(int index)
{
    const int previous = mLocationPage->selectedIndex();
    const Location &location = Locations::all().at(index);

    mLocationPage->setSelectedIndex(index);
    const bool ok = mSettingsModel->setLocation(location.nameEn, location.timeZone);
    mLocationPage->finishApply(previous, ok);

    if (ok) applyLocation(index);
}

void AppWindow::applyLocation(int index)
{
    mLocationPage->setSelectedIndex(index);
    showLocation(index);
    mSystemStatus->refresh();
    mLocationPage->refreshTimes();
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

void AppWindow::showFactoryResetSheet()
{
    mFactoryResetSheet->setBody(tr("Mọi cài đặt sẽ về mặc định. Dữ liệu đo được giữ nguyên."));
    mFactoryResetSheet->present();
}

void AppWindow::onFactoryResetAccepted()
{
    if (!mSettingsModel->reset()) onResetFinished(false);
}

void AppWindow::onResetFinished(bool ok)
{
    if (!ok) {
        mFactoryResetSheet->setBody(tr("Không khôi phục được cài đặt. Hãy thử lại."));
        mFactoryResetSheet->present();
        return;
    }
    onSamplingPeriodRequested(mSettingsModel->samplingPeriod());
}

int AppWindow::sizeMb() const
{
    const qint64 bytes = mOta->availableSize();
    return bytes > 0 ? static_cast<int>(qMax<qint64>(1, bytes / (1000 * 1000))) : 0;
}

void AppWindow::onRolledBack(const QString &failedVersion)
{
    if (failedVersion.isEmpty()) return;

    mUpdateFlow->showRolledBack(mOta->currentVersion(), failedVersion);
    showTakeover(mUpdateFlow);
}
