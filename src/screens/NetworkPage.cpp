#include "screens/NetworkPage.h"

#include "app/Language.h"
#include "app/Theme.h"
#include "widgets/KeyValueList.h"
#include "widgets/ListRow.h"

#include <QFrame>
#include <QVariant>

static constexpr int kCardWidth = 224;
static constexpr int kCardHeight = 51;
static constexpr int kTop = 6;
static constexpr int kGap = 7;

NetworkPage::NetworkPage(QWidget *parent)
    : SubPage(QString(), parent)
    , mList(new KeyValueList(content()))
{
    mEthernetCard = addCard(QStringLiteral("Ethernet"), &mEthernet);
    mWifiCard = addCard(QStringLiteral("Wi-Fi"), &mWifi);

    mServer = mList->addRow(QString());
    mTimeSync = mList->addRow(QString());

    connect(mEthernet, &QAbstractButton::clicked, this, &NetworkPage::ethernetRequested);
    connect(mWifi, &QAbstractButton::clicked, this, &NetworkPage::wifiRequested);

    Language::bind(this, [this]() {
        setTitle(tr("Mạng"));
        mList->setLabel(mServer, tr("Máy chủ"));
        mList->setLabel(mTimeSync, tr("Đồng bộ giờ"));
        refreshTexts();
    });

    layoutCards();
}

QFrame *NetworkPage::addCard(const QString &title, ListRow **row)
{
    auto *card = new QFrame(content());
    card->setProperty("surface", "card");

    const QMargins shadow = Theme::shadowMargins();
    *row = new ListRow(title, card);
    (*row)->setCardStyle(true);
    (*row)->setSeparator(false);
    (*row)->setGeometry(shadow.left(), shadow.top(), kCardWidth, kCardHeight);
    return card;
}

void NetworkPage::layoutCards()
{
    const QMargins shadow = Theme::shadowMargins();

    int y = kTop;
    for (QFrame *card : {mEthernetCard, mWifiCard}) {
        if (card->isHidden()) continue;
        card->setGeometry(QRect(8, y, kCardWidth, kCardHeight).marginsAdded(shadow));
        y += kCardHeight + kGap;
    }
    mList->place(8, y);
}

void NetworkPage::setStatus(const NetworkStatus &status)
{
    mStatus = status;
    mEthernetCard->setVisible(status.ethernet.present);
    mWifiCard->setVisible(status.wifi.present);
    layoutCards();
    refreshTexts();
}

void NetworkPage::setServerConnected(bool connected)
{
    mServerConnected = connected;
    refreshTexts();
}

void NetworkPage::setTimeSynced(bool synced)
{
    mTimeSynced = synced;
    refreshTexts();
}

void NetworkPage::refreshTexts()
{
    const NetworkLink &ethernet = mStatus.ethernet;
    mEthernet->setLeadingIcon(QStringLiteral("network"), ethernet.linkUp ? QStringLiteral("green") : QStringLiteral("default"));
    mEthernet->setSubtitle(!ethernet.linkUp ? tr("Chưa cắm cáp mạng") : ethernet.ip.isEmpty() ? QStringLiteral("--") : ethernet.ip);

    const NetworkLink &wifi = mStatus.wifi;
    mWifi->setLeadingIcon(QStringLiteral("wifi"), wifi.linkUp ? QStringLiteral("blue") : QStringLiteral("default"));
    mWifi->setSubtitle(!wifi.linkUp ? tr("Chưa kết nối mạng nào") : wifi.ssid.isEmpty() ? QStringLiteral("--") : wifi.ssid);

    mList->setValue(mServer, mServerConnected ? tr("Đã kết nối") : tr("Mất kết nối"),
                    mServerConnected ? KeyValueList::Good : KeyValueList::Bad);
    mList->setValue(mTimeSync, mTimeSynced ? tr("Đã đồng bộ") : tr("Chưa đồng bộ"),
                    mTimeSynced ? KeyValueList::Good : KeyValueList::Bad);
}

void NetworkPage::showEvent(QShowEvent *event)
{
    SubPage::showEvent(event);
    emit refreshRequested();
}
