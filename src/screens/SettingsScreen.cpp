#include "screens/SettingsScreen.h"
#include "ui_SettingsScreen.h"

#include <QCheckBox>
#include <QPushButton>
#include <QScroller>
#include <QScrollerProperties>
#include <QSlider>
#include <QTimer>

SettingsScreen::SettingsScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsScreen)
{
    ui->setupUi(this);

    QScroller::grabGesture(ui->settingsScroll->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(ui->settingsScroll->viewport());
    QScrollerProperties props = scroller->scrollerProperties();

    props.setScrollMetric(QScrollerProperties::AxisLockThreshold, 1.0);
    props.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.5);
    props.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0);
    props.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0);

    scroller->setScrollerProperties(props);

    QObject::connect(ui->brightnessSlider, &QSlider::valueChanged, this, &SettingsScreen::onBrightnessChanged);
    QObject::connect(ui->otaToggle, &QCheckBox::toggled, this, &SettingsScreen::onOtaToggled);
    QObject::connect(ui->checkUpdatesButton, &QPushButton::clicked, this, &SettingsScreen::checkForUpdateRequested);

    setSettingsEnabled(false);
    setMacAddress(QString());
}

void SettingsScreen::setOtaMode(bool autoMode)
{
    ui->otaToggle->setChecked(autoMode);
}

void SettingsScreen::setCurrentVersion(const QString &version)
{
    ui->firmwareVersionValue->setText(version);
}

void SettingsScreen::onCheckStarted()
{
    ui->checkUpdatesButton->setEnabled(false);
    ui->checkUpdatesButton->setText("Checking…");
}

void SettingsScreen::onCheckFinished(const QString &resultText)
{
    ui->checkUpdatesButton->setEnabled(true);
    ui->checkUpdatesButton->setText(resultText);
    QTimer::singleShot(2500, ui->checkUpdatesButton, [this]() {
        ui->checkUpdatesButton->setText("Check for updates");
    });
}

SettingsScreen::~SettingsScreen()
{
    delete ui;
}

void SettingsScreen::setBrightness(int value)
{
    ui->brightnessSlider->blockSignals(true);
    ui->brightnessSlider->setValue(value);
    ui->brightnessSlider->blockSignals(false);
    ui->brightnessValue->setText(QString("%1%").arg(value));
}

void SettingsScreen::setSettingsEnabled(bool enabled)
{
    ui->brightnessSlider->setEnabled(enabled);
    ui->otaToggle->setEnabled(enabled);
}

void SettingsScreen::setMacAddress(const QString &mac)
{
    ui->deviceMacValue->setText(mac.isEmpty() ? QStringLiteral("--") : mac);
}

void SettingsScreen::onBrightnessChanged(int value)
{
    ui->brightnessValue->setText(QString("%1%").arg(value));
    emit brightnessChangeRequested(value);
}

void SettingsScreen::onOtaToggled(bool checked)
{
    if (checked) {
        ui->otaModeLabel->setText("Automatic");
        ui->otaDescLabel->setText("Show updates when available");
    } else {
        ui->otaModeLabel->setText("Manual");
        ui->otaDescLabel->setText("Confirm before updating");
    }
    emit otaAutoModeChanged(checked);
}
