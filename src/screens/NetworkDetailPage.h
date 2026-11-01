#ifndef NETWORK_DETAIL_PAGE_H
#define NETWORK_DETAIL_PAGE_H

#include "model/NetworkModel.h"
#include "widgets/SubPage.h"

class KeyValueList;

class NetworkDetailPage : public SubPage
{
    Q_OBJECT

public:
    enum Kind { Ethernet, Wifi };

    explicit NetworkDetailPage(Kind kind, QWidget *parent = nullptr);

public slots:
    void setStatus(const NetworkStatus &status);

signals:
    void refreshRequested();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void refreshTexts();

    Kind mKind;
    KeyValueList *mList;
    int mFirst;
    int mSecond;
    int mIp;
    NetworkLink mLink;
};

#endif // NETWORK_DETAIL_PAGE_H
