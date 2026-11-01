#ifndef SERVICE_PAGE_H
#define SERVICE_PAGE_H

#include "widgets/SubPage.h"

class ListCard;
class ListRow;

class ServicePage : public SubPage
{
    Q_OBJECT

public:
    explicit ServicePage(QWidget *parent = nullptr);

    void setAdvancedVisible(bool visible);

signals:
    void deviceRequested();
    void systemRequested();
    void networkRequested();
    void advancedRequested();
    void factoryResetRequested();

private:
    ListCard *mCard;
    ListRow *mAdvanced;
};

#endif // SERVICE_PAGE_H
