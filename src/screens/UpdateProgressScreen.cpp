#include "screens/UpdateProgressScreen.h"
#include "ui_UpdateProgressScreen.h"

#include <QLabel>
#include <QPushButton>
#include <QStyle>

namespace {

void repolish(QWidget *w)
{
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

constexpr const char *kStatePending = "pending";
constexpr const char *kStateActive  = "active";
constexpr const char *kStateDone    = "done";

} // namespace


UpdateProgressScreen::UpdateProgressScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UpdateProgressScreen)
    , mState(OtaState::Idle)
{
    ui->setupUi(this);

    QObject::connect(ui->actionButton, &QPushButton::clicked, this, &UpdateProgressScreen::onActionClicked);
}

UpdateProgressScreen::~UpdateProgressScreen()
{
    delete ui;
}

void UpdateProgressScreen::setTargetVersion(const QString &version)
{
    mTargetVersion = version;
}

void UpdateProgressScreen::setFailureReason(const QString &reason)
{
    mFailureReason = reason;
}

void UpdateProgressScreen::setState(OtaState state)
{
    mState = state;

    switch (state) {
    case OtaState::Downloading:
        markStep(StepFlash, kStatePending, -1);
        setPhaseHeader("Downloading...", "Fetching firmware from server");
        markStep(StepDownload, kStateActive, 0);
        setActionButton("Cancel", "cancel", /*enabled=*/false);
        break;
    case OtaState::Flashing:
        markStep(StepDownload, kStateDone, -1);
        setPhaseHeader("Flashing...", "Writing to flash · do not power off");
        markStep(StepFlash, kStateActive, 0);
        setActionButton("Cancel", "cancel", /*enabled=*/false);
        break;
    case OtaState::Done:
        markStep(StepFlash, kStateDone, -1);
        setPhaseHeader("Update complete", mTargetVersion + " installed");
        setActionButton("Reboot now", "primary", /*enabled=*/true);
        break;
    case OtaState::Failed:
        setPhaseHeader("Update failed", mFailureReason);
        setActionButton("Back", "cancel", /*enabled=*/true);
        break;
    default:
        break;
    }
}

void UpdateProgressScreen::setProgress(int percent)
{
    if (mState == OtaState::Downloading) {
        markStep(StepDownload, kStateActive, percent);
    } else if (mState == OtaState::Flashing) {
        markStep(StepFlash, kStateActive, percent);
    }
}

void UpdateProgressScreen::onActionClicked()
{
    if (mState == OtaState::Done) {
        setPhaseHeader("Rebooting...", "Device is restarting");
        setActionButton("Rebooting...", "primary", /*enabled=*/false);
        emit rebootRequested();
        return;
    }

    emit cancelled();
}

void UpdateProgressScreen::markStep(Step step, const QString &state, int progress)
{
    QLabel *icon    = nullptr;
    QLabel *label   = nullptr;
    QLabel *percent = nullptr;

    switch (step) {
    case StepDownload:
        icon = ui->downloadIcon; label = ui->downloadLabel; percent = ui->downloadPercent;
        break;
    case StepFlash:
        icon = ui->flashIcon;    label = ui->flashLabel;    percent = ui->flashPercent;
        break;
    }

    setStepState(icon, label, percent, state, progress);
}

void UpdateProgressScreen::setStepState(QLabel *icon, QLabel *label, QLabel *percent,
                                          const QString &state, int progress)
{
    icon->setProperty("state", state);
    label->setProperty("state", state);
    percent->setProperty("state", state);

    icon->setText(state == kStateDone ? QStringLiteral("✓") : QString());

    if (state == kStateActive && progress >= 0) {
        percent->setText(QString::number(progress) + "%");
    } else {
        percent->setText(QString());
    }

    repolish(icon);
    repolish(label);
    repolish(percent);
}

void UpdateProgressScreen::setPhaseHeader(const QString &title, const QString &subtitle)
{
    ui->phaseTitle->setText(title);
    ui->phaseSubtitle->setText(subtitle);
}

void UpdateProgressScreen::setActionButton(const QString &text, const QString &variant, bool enabled)
{
    ui->actionButton->setText(text);
    ui->actionButton->setProperty("variant", variant);
    ui->actionButton->setEnabled(enabled);
    repolish(ui->actionButton);
}
