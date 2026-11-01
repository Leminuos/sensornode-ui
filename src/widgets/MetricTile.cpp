#include "widgets/MetricTile.h"

#include "app/Theme.h"

#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QVariant>

static constexpr int kIconSize = 16;
static constexpr int kHeroWidth = 200;

namespace {

struct Style
{
    const char *property;
    const char *icon;
    const char *text;
    const char *muted;
    const char *line;
    const char *fill;
};

const Style &styleFor(Metric metric)
{
    static const Style temperature{"temperature", "thermometer", "on-temp", "on-temp", "temp-line", "temp-fill"};
    static const Style humidity{"humidity", "droplet", "on-humi", "on-humi", "humi-line", "humi-fill"};
    static const Style light{"light", "sun", "on-light", "on-light-muted", "light-line", "light-fill"};

    switch (metric) {
    case Metric::Temperature: return temperature;
    case Metric::Humidity:    return humidity;
    case Metric::Light:       return light;
    }
    return temperature;
}

} // namespace

MetricTile::MetricTile(Metric metric, const QString &label, QWidget *parent)
    : QAbstractButton(parent)
    , mMetric(metric)
{
    setText(label);
    setFocusPolicy(Qt::NoFocus);
    setProperty("metric", styleFor(metric).property);
    setProperty("dead", true);
    mUnit = metric == Metric::Temperature ? QStringLiteral("°C")
          : metric == Metric::Humidity    ? QStringLiteral("%")
                                          : QStringLiteral("lx");
}

void MetricTile::setUnit(const QString &unit)
{
    mUnit = unit;
    update();
}

void MetricTile::setVisualGeometry(const QRect &rect)
{
    setGeometry(rect.marginsAdded(Theme::shadowMargins()));
    rebuildTrend();
}

void MetricTile::setReading(const QString &value)
{
    if (value == mValue) return;

    const bool wasDead = mValue.isEmpty();
    mValue = value;

    if (wasDead != mValue.isEmpty()) {
        setProperty("dead", mValue.isEmpty());
        style()->unpolish(this);
        style()->polish(this);
    }
    update();
}

void MetricTile::setTrend(const QVector<QPointF> &points)
{
    mTrend = points;
    rebuildTrend();
    update();
}

QRect MetricTile::faceRect() const
{
    return rect().marginsRemoved(Theme::shadowMargins());
}

void MetricTile::rebuildTrend()
{
    mLine = QPainterPath();
    mFill = QPainterPath();
    if (mTrend.size() < 2) return;

    const QRect face = faceRect();
    const bool hero = face.width() >= kHeroWidth;
    const int plotHeight = hero ? 42 : 32;
    const int top = face.bottom() + 1 - plotHeight + 4;
    const int bottom = face.bottom() + 1 - 3;

    double low = mTrend.first().y();
    double high = low;
    for (const QPointF &point : qAsConst(mTrend)) {
        low = qMin(low, point.y());
        high = qMax(high, point.y());
    }

    for (int i = 0; i < mTrend.size(); ++i) {
        const QPointF &point = mTrend.at(i);
        const double ratio = high > low ? (point.y() - low) / (high - low) : 0.5;
        const QPointF mapped(face.left() + point.x() * face.width(), bottom - ratio * (bottom - top));
        if (i == 0) {
            mLine.moveTo(mapped);
        } else {
            mLine.lineTo(mapped);
        }
    }

    mFill = mLine;
    mFill.lineTo(mLine.currentPosition().x(), face.bottom() + 1);
    mFill.lineTo(mLine.elementAt(0).x, face.bottom() + 1);
    mFill.closeSubpath();
}

bool MetricTile::hitButton(const QPoint &pos) const
{
    return faceRect().contains(pos);
}

void MetricTile::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QStyleOption option;
    option.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);

    const Style &tile = styleFor(mMetric);
    const bool dead = mValue.isEmpty();
    const QColor text = Theme::color(dead ? QStringLiteral("text-muted") : QLatin1String(tile.text));
    const QColor muted = Theme::color(dead ? QStringLiteral("text-muted") : QLatin1String(tile.muted));

    const QRect face = faceRect();
    const bool hero = face.width() >= kHeroWidth;
    const int padX = hero ? 10 : 9;
    const int padY = hero ? 8 : 7;

    if (!dead && !mLine.isEmpty()) {
        const int radius = Theme::metric(QStringLiteral("radius-card"));
        QPainterPath clip;
        clip.addRoundedRect(face, radius, radius);

        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setClipPath(clip);
        painter.fillPath(mFill, Theme::color(QLatin1String(tile.fill)));
        painter.setPen(QPen(Theme::color(QLatin1String(tile.line)), 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(mLine);
        painter.restore();
    }

    painter.setPen(muted);
    painter.setFont(Theme::font(QStringLiteral("small")));
    const QRect labelRow(face.left() + padX, face.top() + padY, face.width() - 2 * padX, kIconSize);
    painter.drawText(labelRow, Qt::AlignLeft | Qt::AlignVCenter, this->text());

    const QString iconColor = dead ? QStringLiteral("text-muted") : QLatin1String(tile.muted);
    painter.drawPixmap(labelRow.right() + 1 - kIconSize, labelRow.top(),
                       Theme::icon(QLatin1String(tile.icon), kIconSize, iconColor));

    QFont number = Theme::font(hero ? QStringLiteral("value") : QStringLiteral("value-sm"), true);
    if (hero) {
        number.setLetterSpacing(QFont::AbsoluteSpacing, -1);
    }
    const QString shown = dead ? QStringLiteral("--") : mValue;
    const int lineHeight = hero ? 50 : 30;
    const QRect numberRow(labelRow.left(), labelRow.bottom() + 1, labelRow.width(), lineHeight);

    painter.setPen(text);
    painter.setFont(number);
    painter.drawText(numberRow, Qt::AlignLeft | Qt::AlignVCenter, shown);

    const QFontMetrics numberMetrics(number);
    const int baseline = numberRow.top() + (numberRow.height() + numberMetrics.ascent() - numberMetrics.descent()) / 2;

    painter.setPen(muted);
    painter.setFont(Theme::font(hero ? QStringLiteral("unit") : QStringLiteral("caption"), true));
    painter.drawText(numberRow.left() + numberMetrics.horizontalAdvance(shown) + (hero ? 3 : 2), baseline, mUnit);
}
