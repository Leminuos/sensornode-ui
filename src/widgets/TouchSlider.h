#ifndef TOUCH_SLIDER_H
#define TOUCH_SLIDER_H

#include <QSlider>

class TouchSlider : public QSlider
{
    Q_OBJECT

public:
    explicit TouchSlider(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // TOUCH_SLIDER_H
