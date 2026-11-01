#ifndef UPDATE_PAGE_H
#define UPDATE_PAGE_H

#include "widgets/SubPage.h"

class QCheckBox;
class QLabel;
class QTimer;
class RaisedButton;

class UpdatePage : public SubPage
{
    Q_OBJECT

public:
    explicit UpdatePage(QWidget *parent = nullptr);

public slots:
    void setCurrentVersion(const QString &version);
    void setAutoNotify(bool enabled);
    void setAutoNotifyEnabled(bool enabled);
    void setVerifying(bool verifying);
    void onCheckStarted();
    void onUpToDate();
    void onCheckFailed();
    void onUpdateFound();

signals:
    void autoNotifyRequested(bool enabled);
    void checkRequested();

private:
    void showResult(const QString &text);
    void refreshButton();

    QLabel *mVersion;
    QCheckBox *mToggle;
    RaisedButton *mCheck;
    QTimer *mResultTimer;
    QString mResult;
    bool mChecking = false;
    bool mVerifying = false;
};

#endif // UPDATE_PAGE_H
