#include "widgets/ProgressRing.h"

#include "app/Theme.h"

#include <QPainter>

static constexpr int kSize = 120;
static constexpr int kRadius = 52;
static constexpr int kStroke = 10;

ProgressRing::ProgressRing(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(kSize, kSize);
}

void ProgressRing::setPercent(int percent)
{
    percent = qBound(0, percent, 100);
    if (percent == mPercent) return;

    mPercent = percent;
    update();
}

void ProgressRing::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF circle(kSize / 2.0 - kRadius, kSize / 2.0 - kRadius, 2 * kRadius, 2 * kRadius);

    painter.setPen(QPen(Theme::color(QStringLiteral("track")), kStroke));
    painter.drawEllipse(circle);

    QPen arc(Theme::color(QStringLiteral("primary")), kStroke);
    arc.setCapStyle(Qt::RoundCap);
    painter.setPen(arc);
    painter.drawArc(circle, 90 * 16, -mPercent * 360 * 16 / 100);

    painter.setPen(Theme::color(QStringLiteral("text")));
    painter.setFont(Theme::font(QStringLiteral("ring"), true));
    painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("%1%").arg(mPercent));
}
