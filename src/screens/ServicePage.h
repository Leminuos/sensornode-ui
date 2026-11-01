#ifndef SERVICE_PAGE_H
#define SERVICE_PAGE_H

#include "widgets/SubPage.h"

class ServicePage : public SubPage
{
    Q_OBJECT

public:
    explicit ServicePage(QWidget *parent = nullptr);

signals:
    void firmwareRequested();
    void deviceRequested();
    void sensorsRequested();
    void calibrationRequested();
    void errorLogRequested();
};

#endif // SERVICE_PAGE_H
