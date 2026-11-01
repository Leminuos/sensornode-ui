#include "widgets/StatusHeader.h"

#include "app/Language.h"
#include "app/Theme.h"

#include <QLabel>
#include <QVariant>

static constexpr int kClockWidth = 38;
static constexpr int kNetworkIconSize = 15;
static constexpr int kNetworkIconGap = 4;

static void showIcon(QLabel *label, const QString &glyph, const QString &colorName)
{
    label->setPixmap(Theme::icon(glyph, kNetworkIconSize, colorName));
    label->setProperty("glyph", glyph);
    label->setProperty("colorName", colorName);
}

StatusHeader::StatusHeader(QWidget *parent)
    : QWidget(parent)
    , mClock(new QLabel(this))
    , mChip(new QLabel(this))
    , mEthernet(new QLabel(this))
    , mWifi(new QLabel(this))
{
    const int height = Theme::metric(QStringLiteral("size-status-height"));
    const int inset = Theme::metric(QStringLiteral("space-status-inset"));
    setFixedHeight(height);

    mClock->setObjectName(QStringLiteral("statusClock"));
    mClock->setGeometry(inset, 0, kClockWidth, height);

    mChip->setObjectName(QStringLiteral("systemCheckChip"));
    mChip->setAlignment(Qt::AlignCenter);

    mEthernet->setObjectName(QStringLiteral("statusEthernet"));
    mWifi->setObjectName(QStringLiteral("statusWifi"));
    for (QLabel *icon : {mEthernet, mWifi}) {
        icon->setGeometry(0, (height - kNetworkIconSize) / 2, kNetworkIconSize, kNetworkIconSize);
    }

    setClock(QString());
    setNetworkStatus(NetworkStatus());

    Language::bind(this, [this]() {
        mChip->setText(tr("Kiểm tra hệ thống"));
        setVerifying(mVerifying);
    });
}

void StatusHeader::setClock(const QString &text)
{
    mClock->setText(text.isEmpty() ? QStringLiteral("--:--") : text);
    mClock->setProperty("synced", !text.isEmpty());
    Theme::repolish(mClock);
}

void StatusHeader::setNetworkStatus(const NetworkStatus &status)
{
    const bool online = status.online();
    const bool showEthernet = status.ethernet.present;
    const bool showWifi = status.wifi.present && (online || !showEthernet);
    const QString color = online ? QStringLiteral("text-muted") : QStringLiteral("error");

    int x = 240 - Theme::metric(QStringLiteral("space-status-inset"));
    if (showWifi) {
        x -= kNetworkIconSize;
        mWifi->move(x, mWifi->y());
        x -= kNetworkIconGap;
        showIcon(mWifi,
                 !status.wifi.online()        ? QStringLiteral("wifi_off")
                     : status.wifi.weakSignal() ? QStringLiteral("wifi_weak")
                                                : QStringLiteral("wifi"),
                 color);
    }
    if (showEthernet) {
        mEthernet->move(x - kNetworkIconSize, mEthernet->y());
        showIcon(mEthernet, status.ethernet.online() ? QStringLiteral("network") : QStringLiteral("network_off"),
                 color);
    }
    mWifi->setVisible(showWifi);
    mEthernet->setVisible(showEthernet);
}

void StatusHeader::setVerifying(bool verifying)
{
    mVerifying = verifying;
    if (verifying) {
        mChip->ensurePolished();
        const QSize hint = mChip->sizeHint();
        const int chipWidth = qMin(hint.width(), Theme::metric(QStringLiteral("size-chip-max-width")));
        mChip->setGeometry((240 - chipWidth) / 2, (height() - hint.height()) / 2, chipWidth, hint.height());
    }
    mChip->setVisible(verifying);
}
