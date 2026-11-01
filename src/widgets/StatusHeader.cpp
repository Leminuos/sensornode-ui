#include "widgets/StatusHeader.h"

#include "app/Language.h"
#include "app/Theme.h"

#include <QLabel>
#include <QVariant>

static constexpr int kClockWidth = 38;
static constexpr int kNetworkIconSize = 15;

StatusHeader::StatusHeader(QWidget *parent)
    : QWidget(parent)
    , mClock(new QLabel(this))
    , mChip(new QLabel(this))
    , mNetwork(new QLabel(this))
{
    const int height = Theme::metric(QStringLiteral("size-status-height"));
    const int inset = Theme::metric(QStringLiteral("space-status-inset"));
    setFixedHeight(height);

    mClock->setObjectName(QStringLiteral("statusClock"));
    mClock->setGeometry(inset, 0, kClockWidth, height);

    mChip->setObjectName(QStringLiteral("systemCheckChip"));
    mChip->setAlignment(Qt::AlignCenter);

    mNetwork->setGeometry(240 - inset - kNetworkIconSize, (height - kNetworkIconSize) / 2,
                          kNetworkIconSize, kNetworkIconSize);

    setClock(QString());
    setNetworkUp(false);

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

void StatusHeader::setNetworkUp(bool up)
{
    mNetwork->setPixmap(up ? Theme::icon(QStringLiteral("network"), kNetworkIconSize, QStringLiteral("text-muted"))
                           : Theme::icon(QStringLiteral("network_off"), kNetworkIconSize, QStringLiteral("error")));
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
