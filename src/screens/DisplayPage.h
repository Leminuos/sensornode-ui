#ifndef DISPLAY_PAGE_H
#define DISPLAY_PAGE_H

#include "widgets/SubPage.h"

class QLabel;
class SegmentedControl;
class TouchSlider;

class DisplayPage : public SubPage
{
    Q_OBJECT

public:
    explicit DisplayPage(QWidget *parent = nullptr);

public slots:
    void setBrightness(int value);
    void setBrightnessEnabled(bool enabled);
    void setAutoDimMinutes(int minutes);

signals:
    void brightnessPreviewed(int value);
    void brightnessCommitted(int value);
    void autoDimRequested(int minutes);

private:
    TouchSlider *mSlider;
    QLabel *mValue;
    SegmentedControl *mAutoDim;
};

#endif // DISPLAY_PAGE_H
