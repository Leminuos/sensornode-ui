#ifndef UPDATE_FLOW_PAGE_H
#define UPDATE_FLOW_PAGE_H

#include <QWidget>

class ProgressRing;
class QLabel;
class RaisedButton;
class StepBar;

class UpdateFlowPage : public QWidget
{
    Q_OBJECT

public:
    enum class View { Downloading, Installing, Installed, Rebooting, DownloadFailed, InstallFailed, RolledBack };

    explicit UpdateFlowPage(QWidget *parent = nullptr);

    View view() const { return mView; }

    void showDownloading(const QString &version, int sizeMb);
    void showInstalling();
    void showInstalled(const QString &version, bool rebootFailed);
    void showRebooting();
    void showDownloadFailed();
    void showInstallFailed(const QString &currentVersion);
    void showRolledBack(const QString &currentVersion, const QString &failedVersion);

public slots:
    void setProgress(int percent);

signals:
    void rebootRequested();
    void retryRequested();
    void closeRequested();
    void rollbackAcknowledged();

private:
    enum class Actions { None, Reboot, RebootLocked, CloseRetry, Acknowledge };

    void showProgressLayout(bool warning);
    void showResultLayout(bool danger, const QString &glyph, const QString &title, const QString &body,
                          bool bodyIsError = false);
    void setActions(Actions actions);

    View mView = View::Downloading;
    ProgressRing *mRing;
    QLabel *mProgressTitle;
    QLabel *mProgressNote;
    QLabel *mPowerWarning;
    QLabel *mHalo;
    QLabel *mResultTitle;
    QLabel *mResultBody;
    StepBar *mSteps;
    RaisedButton *mPrimary;
    RaisedButton *mSecondary;
};

#endif // UPDATE_FLOW_PAGE_H
