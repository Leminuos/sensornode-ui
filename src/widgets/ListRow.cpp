#include "widgets/ListRow.h"

#include "app/Theme.h"

#include <QPainter>

static constexpr int kPadding = 8;
static constexpr int kGap = 8;
static constexpr int kTileSize = 26;
static constexpr int kTileTextX = 42;
static constexpr int kOptionTextX = 12;
static constexpr int kChevronSize = 14;
static constexpr int kCheckSize = 18;

ListRow::ListRow(const QString &title, QWidget *parent)
    : QAbstractButton(parent)
{
    setText(title);
    setFocusPolicy(Qt::NoFocus);
}

void ListRow::setLeadingIcon(const QString &glyph, const QString &tile)
{
    mGlyph = glyph;
    mTile = tile;
    update();
}

void ListRow::setTitle(const QString &title)
{
    setText(title);
    update();
}

void ListRow::setSubtitle(const QString &subtitle)
{
    mSubtitle = subtitle;
    update();
}

void ListRow::setValue(const QString &value)
{
    mValue = value;
    update();
}

void ListRow::setTrailing(Trailing trailing)
{
    mTrailing = trailing;
    update();
}

void ListRow::setSelected(bool selected)
{
    mSelected = selected;
    update();
}

void ListRow::setClockText(const QString &text)
{
    mClock = text;
    update();
}

void ListRow::setSeparator(bool visible)
{
    mSeparator = visible;
    update();
}

void ListRow::setPressable(bool pressable)
{
    mPressable = pressable;
    update();
}

void ListRow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    if (isDown() && mPressable) {
        painter.fillRect(rect(), Theme::color(QStringLiteral("track")));
    }

    const bool hasTile = !mTile.isEmpty();
    const int textX = hasTile ? kTileTextX : kOptionTextX;

    if (mSeparator) {
        painter.fillRect(QRect(textX, 0, width() - textX, 1), Theme::color(QStringLiteral("divider")));
    }

    if (hasTile) {
        const QPixmap tile(QStringLiteral(":/surfaces/icon_tile_%1.png").arg(mTile));
        const int tileY = (height() - tile.height()) / 2;
        painter.drawPixmap(kPadding - 2, tileY, tile);

        const QString tint = mTile == QLatin1String("yellow") || mTile == QLatin1String("aqua")
                                     || mTile == QLatin1String("green")
                                 ? QStringLiteral("on-icon-tile-") + mTile
                                 : QStringLiteral("on-icon-tile");
        painter.drawPixmap(kPadding + (kTileSize - 16) / 2, tileY + 1 + (kTileSize - 16) / 2,
                           Theme::icon(mGlyph, 16, tint));
    }

    const bool muted = !isEnabled();
    const QColor textColor = Theme::color(muted ? QStringLiteral("text-muted") : QStringLiteral("text"));

    int right = width() - kPadding;
    if (mTrailing == Chevron) {
        right -= kChevronSize;
        painter.drawPixmap(right, (height() - kChevronSize) / 2,
                           Theme::icon(QStringLiteral("chevron"), kChevronSize, QStringLiteral("text-muted")));
        right -= kGap;
    } else if (mTrailing == CheckMark) {
        if (!mClock.isEmpty()) {
            right -= 6;
        }
        right -= kCheckSize;
        if (mSelected) {
            painter.drawPixmap(right, (height() - kCheckSize) / 2,
                               Theme::icon(QStringLiteral("check"), kCheckSize, QStringLiteral("primary")));
        }
        right -= kGap;
    }

    if (!mClock.isEmpty()) {
        painter.setFont(Theme::font(QStringLiteral("body"), true));
        painter.setPen(Theme::color(mSelected ? QStringLiteral("primary") : QStringLiteral("text")));
        const int clockWidth = painter.fontMetrics().horizontalAdvance(mClock);
        painter.drawText(QRect(right - clockWidth, 0, clockWidth, height()), Qt::AlignVCenter, mClock);
        right -= clockWidth + kGap;
    }

    if (!mValue.isEmpty()) {
        painter.setFont(Theme::font(QStringLiteral("small")));
        painter.setPen(Theme::color(QStringLiteral("text-muted")));
        const int valueWidth = painter.fontMetrics().horizontalAdvance(mValue);
        painter.drawText(QRect(right - valueWidth, 0, valueWidth, height()), Qt::AlignVCenter, mValue);
        right -= valueWidth + kGap;
    }

    painter.setFont(Theme::font(QStringLiteral("body"), mSelected));
    painter.setPen(textColor);
    if (mSubtitle.isEmpty()) {
        painter.drawText(QRect(textX, 0, right - textX, height()), Qt::AlignVCenter, text());
    } else {
        const int titleHeight = painter.fontMetrics().height();
        painter.setFont(Theme::font(QStringLiteral("caption")));
        const int subtitleHeight = painter.fontMetrics().height();
        const int top = (height() - titleHeight - subtitleHeight) / 2;

        painter.setPen(Theme::color(QStringLiteral("text-muted")));
        painter.drawText(QRect(textX, top + titleHeight, right - textX, subtitleHeight), Qt::AlignVCenter, mSubtitle);

        painter.setFont(Theme::font(QStringLiteral("body"), mSelected));
        painter.setPen(textColor);
        painter.drawText(QRect(textX, top, right - textX, titleHeight), Qt::AlignVCenter, text());
    }
}
