#include "widgets/StepBar.h"

#include "app/Theme.h"

#include <QPainter>

static constexpr int kCircle = 18;
static constexpr int kLineY = 8;
static constexpr int kLineHeight = 2;
static constexpr int kLabelY = 21;
static constexpr int kGlyphSize = 12;

StepBar::StepBar(const QStringList &labels, QWidget *parent)
    : QWidget(parent)
    , mLabels(labels)
    , mStates(labels.size(), Pending)
{
    setFixedHeight(kLabelY + 14);
}

void StepBar::setStates(const QVector<StepState> &states)
{
    mStates = states;
    update();
}

void StepBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    const int count = mLabels.size();
    const qreal cell = qreal(width()) / count;
    const auto center = [cell](int i) { return int(cell * i + cell / 2); };

    for (int i = 1; i < count; ++i) {
        const bool finished = mStates.at(i - 1) == Done;
        painter.fillRect(QRect(center(i - 1), kLineY, center(i) - center(i - 1), kLineHeight),
                         Theme::color(finished ? QStringLiteral("ok") : QStringLiteral("track")));
    }

    painter.setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < count; ++i) {
        const StepState state = mStates.at(i);
        const QRect dot(center(i) - kCircle / 2, 0, kCircle, kCircle);

        const char *fill = state == Done ? "ok" : state == Active ? "primary" : state == Failed ? "error" : "track";
        painter.setPen(Qt::NoPen);
        painter.setBrush(Theme::color(QLatin1String(fill)));
        painter.drawEllipse(dot);

        if (state == Done || state == Failed) {
            const QString tint = state == Done ? QStringLiteral("track") : QStringLiteral("on-danger");
            const QString glyph = state == Done ? QStringLiteral("check") : QStringLiteral("close");
            painter.drawPixmap(dot.center().x() - kGlyphSize / 2 + 1, dot.center().y() - kGlyphSize / 2 + 1,
                               Theme::icon(glyph, kGlyphSize, tint));
        } else {
            painter.setPen(Theme::color(state == Active ? QStringLiteral("track") : QStringLiteral("text-muted")));
            painter.setFont(Theme::font(QStringLiteral("caption"), state == Active));
            painter.drawText(dot, Qt::AlignCenter, QString::number(i + 1));
        }

        const bool strong = state == Active || state == Failed;
        painter.setFont(Theme::font(QStringLiteral("caption"), strong));
        painter.setPen(Theme::color(state == Failed   ? QStringLiteral("error")
                                    : state == Active ? QStringLiteral("text")
                                                      : QStringLiteral("text-muted")));
        painter.drawText(QRect(center(i) - int(cell) / 2 - 8, kLabelY, int(cell) + 16, 14), Qt::AlignCenter,
                         mLabels.at(i));
    }
}
