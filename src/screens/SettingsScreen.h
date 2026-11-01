#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include <QWidget>

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
    void setBrightness(int value);
    void setSettingsEnabled(bool enabled);
    void setMacAddress(const QString &mac);

signals:
    void brightnessChangeRequested(int value);
    void otaAutoModeChanged(bool autoMode);  // toggle Auto/Manual
    void checkForUpdateRequested();          // nút "Check for updates"

private:
    void onBrightnessChanged(int value);
    void onOtaToggled(bool checked);

    Ui::SettingsScreen *ui;
};

#endif // SETTINGS_SCREEN_H
