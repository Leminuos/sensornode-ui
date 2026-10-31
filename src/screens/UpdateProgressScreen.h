#ifndef UPDATE_PROGRESS_SCREEN_H
#define UPDATE_PROGRESS_SCREEN_H

#include <QString>
#include <QWidget>

#include "model/OtaState.h"

QT_BEGIN_NAMESPACE
namespace Ui { class UpdateProgressScreen; }
QT_END_NAMESPACE

class QLabel;

class UpdateProgressScreen : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateProgressScreen(QWidget *parent = nullptr);
    ~UpdateProgressScreen();

    void setTargetVersion(const QString &version);
    void setFailureReason(const QString &reason);

public slots:
    void setState(OtaState state);
    void setProgress(int percent);

signals:
    void cancelled();
    void rebootRequested();

private:
    void onActionClicked();

    enum Step { StepDownload, StepFlash };

    void markStep(Step step, const QString &state, int progress);
    void setStepState(QLabel *icon, QLabel *label, QLabel *percent, const QString &state, int progress);
    void setPhaseHeader(const QString &title, const QString &subtitle);
    void setActionButton(const QString &text, const QString &variant, bool enabled);

    Ui::UpdateProgressScreen *ui;
    OtaState mState;
    QString mTargetVersion;
    QString mFailureReason;
};

#endif // UPDATE_PROGRESS_SCREEN_H
