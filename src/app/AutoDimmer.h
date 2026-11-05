#ifndef AUTO_DIMMER_H
#define AUTO_DIMMER_H

#include <QObject>
#include <QTimer>

// Tắt màn hình khi không có thao tác trong N phút. Lần chạm đánh thức bị nuốt để không bấm nhầm.
class AutoDimmer : public QObject
{
    Q_OBJECT

public:
    explicit AutoDimmer(QObject *parent = nullptr);

    bool isOff() const { return mOff; }

public slots:
    // 0 là không bao giờ tắt.
    void setTimeoutMinutes(int minutes);
    void setTimeoutMs(int ms);
    void setInhibited(bool inhibited);
    void wake();

signals:
    void offChanged(bool off);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void restart();
    void setOff(bool off);

    QTimer mTimer;
    int mTimeoutMs = 0;
    bool mInhibited = false;
    bool mOff = false;
    bool mSwallowing = false;
};

#endif // AUTO_DIMMER_H
