#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include "app/PageFrame.h"
#include "model/CalibrationFile.h"
#include "model/Metric.h"
#include "model/OtaState.h"

#include <QPointF>
#include <QVector>

class AdvancedPage;
class AutoDimmer;
class BottomSheet;
class CalibrationIntroPage;
class CalibrationPage;
class ChartScreen;
class DeviceInfoModel;
class DevicePage;
class DisplayPage;
class ErrorLogModel;
class ErrorLogPage;
class FirmwarePage;
class DisplayModel;
class HistoryModel;
class HomeScreen;
class LanguageLocationPage;
class LocationPage;
class NetworkDetailPage;
class NetworkModel;
class NetworkPage;
class OtaModel;
class PowerModel;
class SamplingPage;
class SensorsPage;
class ServicePage;
class SettingsModel;
class SettingsScreen;
class SubPage;
class SystemPage;
class SystemStatusModel;
class TelemetryModel;
class QLabel;
class UpdateFlowPage;
class UpdatePage;

class AppWindow : public PageFrame
{
    Q_OBJECT

public:
    explicit AppWindow(QWidget *parent = nullptr);
    ~AppWindow();

private slots:
    void onCheckForUpdateRequested();
    void onCheckFailed(const QString &reason);
    void onSamplingPeriodRequested(int seconds);
    void onTelemetryIntervalChanged(int intervalMs);
    void onIntervalApplied(bool ok);
    void onTemperatureUnitRequested(TemperatureUnit unit);
    void onLanguageRequested(const QString &language);
    void onLocationRequested(int index);
    void onCalibrationPoints(const QVector<QPointF> &touched);
    void onUpdateAvailable(const QString &current, const QString &available);
    void onOtaStateChanged(OtaState state);
    void onUpdateClosed();
    void onRolledBack(const QString &failedVersion);
    void onFactoryResetAccepted();
    void onResetFinished(bool ok);
    void applyTemperatureUnit(TemperatureUnit unit);
    void applyLanguage(const QString &language);

private:
    void buildUi();
    void wireTelemetry();
    void wireSettings();
    void wireSystemStatus();
    void wireService();
    void wireOta();
    void start();

    void buildSettingsPages();
    void buildServicePages();
    void showLocation(int index);
    void applyLocation(int index);
    void buildUpdateFlow();
    void buildCalibration();
    void startCalibration();
    void finishCalibration();
    void revertCalibration();
    void restartForCalibration();
    void applyLanguageTexts();
    void showUpdateSheet();
    void showFactoryResetSheet();
    int sizeMb() const;

    TelemetryModel *mTelemetry;
    HistoryModel *mHistory;
    OtaModel *mOta;
    SettingsModel *mSettingsModel;
    DisplayModel *mDisplayModel;
    AutoDimmer *mAutoDimmer;
    PowerModel *mPower;
    CalibrationFile mCalibrationFile;
    SystemStatusModel *mSystemStatus;
    NetworkModel *mNetwork;
    DeviceInfoModel *mDeviceInfo;
    ErrorLogModel *mErrorLog;

    BottomSheet *mUpdateSheet;
    QLabel *mSheetCurrent;
    QLabel *mSheetNext;
    QString mUpdateVersion;

    HomeScreen *mHome;
    ChartScreen *mCharts;
    SettingsScreen *mSettings;
    DisplayPage *mDisplayPage;
    SamplingPage *mSamplingPage;
    LanguageLocationPage *mLanguagePage;
    LocationPage *mLocationPage;
    UpdatePage *mUpdatePage;
    ServicePage *mServicePage;
    DevicePage *mDevicePage;
    SystemPage *mSystemPage;
    NetworkPage *mNetworkPage;
    NetworkDetailPage *mEthernetPage;
    NetworkDetailPage *mWifiPage;
    AdvancedPage *mAdvancedPage;
    FirmwarePage *mFirmwarePage;
    SensorsPage *mSensorsPage;
    ErrorLogPage *mErrorLogPage;
    CalibrationIntroPage *mCalibrationIntro;
    CalibrationPage *mCalibration;
    BottomSheet *mRestartSheet;
    BottomSheet *mFactoryResetSheet;
    UpdateFlowPage *mUpdateFlow;
};

#endif // APP_WINDOW_H
