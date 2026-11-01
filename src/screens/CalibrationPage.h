#ifndef CALIBRATION_PAGE_H
#define CALIBRATION_PAGE_H

#include <QPointF>
#include <QTimer>
#include <QVector>
#include <QWidget>

class QLabel;
class RaisedButton;

class CalibrationPage : public QWidget
{
    Q_OBJECT

public:
    enum class Step { Idle, Points, Verify };

    explicit CalibrationPage(QWidget *parent = nullptr);

    static const QVector<QPoint> &targets();

    Step step() const { return mStep; }
    int currentPoint() const { return mTouched.size(); }
    int secondsLeft() const { return mSecondsLeft; }

public slots:
    void beginPoints();
    void beginVerify();

signals:
    void pointsCaptured(const QVector<QPointF> &touched);
    void cancelled();
    void kept();
    void reverted();
    void restartRequested();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void showVerifyWidgets(bool visible);
    void updateCountdown();

    Step mStep = Step::Idle;
    QVector<QPointF> mTouched;
    int mSecondsLeft = 0;
    QTimer mIdleTimer;
    QTimer mCountdown;

    QLabel *mQuestion;
    RaisedButton *mKeep;
    QLabel *mCountdownLabel;
    RaisedButton *mAgain;
};

#endif // CALIBRATION_PAGE_H
