#ifndef PROGRESS_RING_H
#define PROGRESS_RING_H

#include <QWidget>

class ProgressRing : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressRing(QWidget *parent = nullptr);

    int percent() const { return mPercent; }

public slots:
    void setPercent(int percent);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int mPercent = 0;
};

#endif // PROGRESS_RING_H
