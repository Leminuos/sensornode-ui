#ifndef CALIBRATION_INTRO_PAGE_H
#define CALIBRATION_INTRO_PAGE_H

#include "widgets/SubPage.h"

class CalibrationIntroPage : public SubPage
{
    Q_OBJECT

public:
    explicit CalibrationIntroPage(QWidget *parent = nullptr);

signals:
    void startRequested();
};

#endif // CALIBRATION_INTRO_PAGE_H
