#include "widgets/TouchSlider.h"

#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionSlider>

static constexpr int kKnobInset = 12;

TouchSlider::TouchSlider(QWidget *parent)
    : QSlider(Qt::Horizontal, parent)
{
    setFocusPolicy(Qt::NoFocus);
}

void TouchSlider::mousePressEvent(QMouseEvent *event)
{
    const int travel = width() - 2 * kKnobInset;
    setValue(QStyle::sliderValueFromPosition(minimum(), maximum(), event->x() - kKnobInset, travel));
    QSlider::mousePressEvent(event);
}
