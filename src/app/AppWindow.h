#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include "app/PageFrame.h"
#include "model/Metric.h"
#include "model/OtaState.h"

#include <QPointF>
#include <QVector>

class BootHoldDetector;
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
class OtaModel;
class SamplingPage;
class SensorsPage;
class ServicePage;
class SettingsModel;
class SettingsScreen;
class SubPage;
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
    void onTemperatureUnitRequested(TemperatureUnit unit);
    void onLanguageRequested(const QString &language);
    void onLocationRequested(int index);
    void onCalibrationPoints(const QVector<QPointF> &touched);
    void onUpdateAvailable(const QString &current, const QString &available);
    void onOtaStateChanged(OtaState state);
    void onUpdateClosed();

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
    void buildUpdateFlow();
    void buildCalibration();
    void startCalibration();
    void finishCalibration();
    void applyLanguageTexts();
    void showUpdateSheet();
    void checkRollback();
    int sizeMb() const;

    TelemetryModel *mTelemetry;
    HistoryModel *mHistory;
    OtaModel *mOta;
    SettingsModel *mSettingsModel;
    DisplayModel *mDisplayModel;
    SystemStatusModel *mSystemStatus;
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
    FirmwarePage *mFirmwarePage;
    DevicePage *mDevicePage;
    SensorsPage *mSensorsPage;
    ErrorLogPage *mErrorLogPage;
    CalibrationIntroPage *mCalibrationIntro;
    CalibrationPage *mCalibration;
    BootHoldDetector *mBootHold;
    BottomSheet *mRestartSheet;
    UpdateFlowPage *mUpdateFlow;
};

#endif // APP_WINDOW_H
