#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <QWidget>

#include "model/OtaState.h"

class BottomNavBar;
class ChartScreen;
class HistoryModel;
class HomeScreen;
class OtaModel;
class QStackedWidget;
class SettingsScreen;
class StatusHeader;
class TelemetryModel;
class UpdatePopup;
class UpdateProgressScreen;

class AppWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AppWindow(QWidget *parent = nullptr);
    ~AppWindow();

private slots:
    void onNavRequested(int tab);
    void onCheckForUpdateRequested();
    void onCheckFailed(const QString &reason);
    void onUpdateAvailable(const QString &current, const QString &available);
    void onOtaStateChanged(OtaState state);
    void onUpdateCancelled();

private:
    void buildUi();
    void wireNavigation();
    void wireTelemetry();
    void wireOta();
    void start();

    void showUpdatePopup();
    void enterUpdateMode();
    void exitUpdateMode();

    TelemetryModel *mTelemetry;
    HistoryModel *mHistory;
    OtaModel *mOta;

    StatusHeader *mStatusHeader;
    QStackedWidget *mStack;
    BottomNavBar *mNavBar;
    UpdatePopup *mPopup;

    HomeScreen *mHome;
    ChartScreen *mCharts;
    SettingsScreen *mSettings;
    UpdateProgressScreen *mProgress;
};

#endif // APP_WINDOW_H
