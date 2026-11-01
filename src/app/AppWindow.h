#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include "app/PageFrame.h"
#include "model/OtaState.h"

class ChartScreen;
class DisplayModel;
class HistoryModel;
class HomeScreen;
class OtaModel;
class SettingsModel;
class SettingsScreen;
class SystemStatusModel;
class TelemetryModel;
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
    UpdateProgressScreen *mProgress;
};

#endif // APP_WINDOW_H
