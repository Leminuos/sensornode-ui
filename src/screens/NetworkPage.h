#ifndef NETWORK_PAGE_H
#define NETWORK_PAGE_H

#include "model/NetworkModel.h"
#include "widgets/SubPage.h"

class KeyValueList;
class ListRow;
class QFrame;

class NetworkPage : public SubPage
{
    Q_OBJECT

public:
    explicit NetworkPage(QWidget *parent = nullptr);

public slots:
    void setStatus(const NetworkStatus &status);
    void setServerConnected(bool connected);
    void setTimeSynced(bool synced);

signals:
    void ethernetRequested();
    void wifiRequested();
    void refreshRequested();

protected:
    void showEvent(QShowEvent *event) override;

private:
    QFrame *addCard(const QString &title, ListRow **row);
    void layoutCards();
    void refreshTexts();

    QFrame *mEthernetCard;
    QFrame *mWifiCard;
    ListRow *mEthernet;
    ListRow *mWifi;
    KeyValueList *mList;
    int mServer;
    int mTimeSync;
    NetworkStatus mStatus;
    bool mServerConnected = false;
    bool mTimeSynced = false;
};

#endif // NETWORK_PAGE_H
