#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include "app/PageFrame.h"
#include "model/Metric.h"
#include "model/OtaState.h"

class ChartScreen;
class DisplayPage;
class DisplayModel;
class HistoryModel;
class HomeScreen;
class LanguageLocationPage;
class LocationPage;
class OtaModel;
class SamplingPage;
class SettingsModel;
class SettingsScreen;
class SubPage;
class SystemStatusModel;
class TelemetryModel;
class UpdatePage;
class UpdatePopup;
class UpdateProgressScreen;

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
    void onUpdateAvailable(const QString &current, const QString &available);
    void onOtaStateChanged(OtaState state);
    void onUpdateCancelled();

private:
    void buildUi();
    void wireTelemetry();
    void wireSettings();
    void wireSystemStatus();
    void wireOta();
    void start();

    void buildSettingsPages();
    void showLocation(int index);
    void showUpdatePopup();
    void enterUpdateMode();
    void exitUpdateMode();

    TelemetryModel *mTelemetry;
    HistoryModel *mHistory;
    OtaModel *mOta;
    SettingsModel *mSettingsModel;
    DisplayModel *mDisplayModel;
    SystemStatusModel *mSystemStatus;

    UpdatePopup *mPopup;

    HomeScreen *mHome;
    ChartScreen *mCharts;
    SettingsScreen *mSettings;
    DisplayPage *mDisplayPage;
    SamplingPage *mSamplingPage;
    LanguageLocationPage *mLanguagePage;
    LocationPage *mLocationPage;
    UpdatePage *mUpdatePage;
    SubPage *mServicePage;
    UpdateProgressScreen *mProgress;
};

#endif // APP_WINDOW_H
