#include "screens/SensorsPage.h"

#include "app/Language.h"
#include "app/Theme.h"

#include <QDateTime>
#include <QFrame>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QVariant>

static constexpr int kCardHeight = 52;
static constexpr int kTickMs = 1000;

SensorsPage::SensorsPage(QWidget *parent)
    : SubPage(QString(), parent)
    , mTick(new QTimer(this))
{
    mSht30 = makeCard(6, QStringLiteral("SHT30"), QStringLiteral("thermometer"), QStringLiteral("red"));
    mBh1750 = makeCard(65, QStringLiteral("BH1750"), QStringLiteral("sun"), QStringLiteral("yellow"));

    QWidget *body = content();

    auto *card = new QFrame(body);
    card->setProperty("surface", "card");
    card->setGeometry(QRect(8, 124, 224, 36).marginsAdded(Theme::shadowMargins()));

    auto *caption = new QLabel(body);
    caption->setObjectName(QStringLiteral("fieldCaption"));
    caption->setGeometry(18, 134, 130, 16);

    mPeriod = new QLabel(QStringLiteral("--"), body);
    mPeriod->setObjectName(QStringLiteral("fieldTitle"));
    mPeriod->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mPeriod->setGeometry(148, 134, 74, 16);

    mTick->setInterval(kTickMs);
    connect(mTick, &QTimer::timeout, this, &SensorsPage::refresh);

    Language::bind(this, [=]() {
        setTitle(tr("Cảm biến"));
        caption->setText(tr("Chu kỳ đang chạy"));
        refresh();
    });
}

SensorsPage::Card SensorsPage::makeCard(int y, const QString &name, const QString &glyph, const QString &tile)
{
    QWidget *body = content();

    auto *frame = new QFrame(body);
    frame->setProperty("surface", "card");
    frame->setGeometry(QRect(8, y, 224, kCardHeight).marginsAdded(Theme::shadowMargins()));

    QPixmap icon(QStringLiteral(":/surfaces/icon_tile_%1.png").arg(tile));
    {
        QPainter painter(&icon);
        const QString tint = tile == QLatin1String("yellow") ? QStringLiteral("on-icon-tile-yellow")
                                                             : QStringLiteral("on-icon-tile");
        painter.drawPixmap(7, 6, Theme::icon(glyph, 16, tint));
    }
    auto *iconLabel = new QLabel(body);
    iconLabel->setPixmap(icon);
    iconLabel->setGeometry(16, y + 11, icon.width(), icon.height());

    auto *title = new QLabel(name, body);
    title->setObjectName(QStringLiteral("fieldTitle"));
    title->setGeometry(52, y + 10, 100, 18);

    Card card;
    card.chip = new QLabel(body);
    card.chip->setAlignment(Qt::AlignCenter);
    card.chip->setProperty("chip", "neutral");

    card.note = new QLabel(body);
    card.note->setObjectName(QStringLiteral("fieldNote"));
    card.note->setGeometry(52, y + 28, 170, 14);
    return card;
}

void SensorsPage::setReport(const SensorReport &report)
{
    mReport = report;
    mHasReport = true;
    refresh();
}

void SensorsPage::setPeriodText(const QString &text)
{
    mPeriod->setText(text);
}

void SensorsPage::setTemperatureUnit(TemperatureUnit unit)
{
    mUnit = unit;
    refresh();
}

void SensorsPage::showEvent(QShowEvent *event)
{
    SubPage::showEvent(event);
    refresh();
    mTick->start();
}

void SensorsPage::hideEvent(QHideEvent *event)
{
    SubPage::hideEvent(event);
    mTick->stop();
}

QString SensorsPage::ageText() const
{
    const qint64 seconds = qMax<qint64>(0, (QDateTime::currentMSecsSinceEpoch() - mReport.timestampMs) / 1000);
    return seconds < 60 ? tr("%1 giây trước").arg(seconds) : tr("%1 phút trước").arg(seconds / 60);
}

void SensorsPage::applyCard(const Card &card, const QString &error, const QString &value)
{
    if (!mHasReport) {
        card.chip->setText(QStringLiteral("--"));
        card.chip->setProperty("chip", "neutral");
        card.note->setText(tr("Chưa có dữ liệu"));
    } else if (!error.isEmpty()) {
        card.chip->setText(tr("Lỗi"));
        card.chip->setProperty("chip", "warn");
        card.note->setText(error);
    } else {
        card.chip->setText(tr("Tốt"));
        card.chip->setProperty("chip", "ok");
        card.note->setText(value);
    }

    Theme::repolish(card.chip);
    card.chip->ensurePolished();
    const QSize hint = card.chip->sizeHint();
    const int y = card.note->y() - 18 + (18 - hint.height()) / 2;
    card.chip->setGeometry(222 - hint.width(), y, hint.width(), hint.height());
}

void SensorsPage::refresh()
{
    const QString age = ageText();
    const QString unit = mUnit == TemperatureUnit::Fahrenheit ? QStringLiteral("°F") : QStringLiteral("°C");

    applyCard(mSht30, mReport.sht30Error,
              QStringLiteral("%1%2, %3%, %4")
                  .arg(displayTemperature(mReport.tempC, mUnit), 0, 'f', 1)
                  .arg(unit)
                  .arg(mReport.humidity, 0, 'f', 0)
                  .arg(age));
    applyCard(mBh1750, mReport.bh1750Error,
              tr("%1 lx, đọc %2").arg(mReport.lux, 0, 'f', 0).arg(age));
}
