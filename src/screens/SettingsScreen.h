#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include <QWidget>

#include "drivers/BacklightPwm.h"
#include "settings/AppSettings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsScreen; }
QT_END_NAMESPACE

class SettingsScreen : public QWidget
{
    Q_OBJECT

public:
    SettingsScreen(QWidget *parent = nullptr);
    ~SettingsScreen();

public slots:
    // Đồng bộ trạng thái toggle Auto/Manual với cấu hình đã lưu (/data).
    void setOtaMode(bool autoMode);
    // Hiển thị version firmware hiện tại (đọc từ /etc/sw-versions qua OtaManager).
    void setCurrentVersion(const QString &version);
    // Phản hồi cho nút "Check for updates" (mode manual).
    void onCheckStarted();
    void onCheckFinished(const QString &resultText);

signals:
    void otaAutoModeChanged(bool autoMode);  // toggle Auto/Manual
    void checkForUpdateRequested();          // nút "Check for updates"

private:
    // Cập nhật nhãn + duty cycle PWM theo giá trị slider (không ghi file).
    void applyBrightness(int value);
    void onBrightnessChanged(int value);
    void onBrightnessReleased();
    void onOtaToggled(bool checked);

    Ui::SettingsScreen *ui;
    BacklightPwm mBacklightPwm;
    AppSettings mAppSettings;
};

#endif // SETTINGS_SCREEN_H
