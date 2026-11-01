#ifndef ADVANCED_PAGE_H
#define ADVANCED_PAGE_H

#include "widgets/SubPage.h"

class AdvancedPage : public SubPage
{
    Q_OBJECT

public:
    explicit AdvancedPage(QWidget *parent = nullptr);

signals:
    void firmwareRequested();
    void sensorsRequested();
    void calibrationRequested();
    void errorLogRequested();
};

#endif // ADVANCED_PAGE_H
