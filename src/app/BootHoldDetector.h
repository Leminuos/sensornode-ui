#ifndef BOOT_HOLD_DETECTOR_H
#define BOOT_HOLD_DETECTOR_H

#include <QObject>
#include <QTimer>

// Backdoor khi cảm ứng lệch tới mức không bấm trúng nút: chạm từ lúc mới khởi động và giữ đủ lâu.
class BootHoldDetector : public QObject
{
    Q_OBJECT

public:
    explicit BootHoldDetector(QObject *parent = nullptr, int windowMs = 5000, int holdMs = 5000);

    void start();

signals:
    void triggered();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void stop();

    QTimer mWindow;
    QTimer mHold;
};

#endif // BOOT_HOLD_DETECTOR_H
