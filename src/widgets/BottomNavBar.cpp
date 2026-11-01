#include "widgets/BottomNavBar.h"

#include "app/Language.h"
#include "app/Theme.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QPainter>

static constexpr int kIconSize = 22;
static const QSize kPillSize(56, 36);

namespace {

class NavButton : public QAbstractButton
{
public:
    NavButton(const QString &icon, QWidget *parent)
        : QAbstractButton(parent)
        , mIcon(icon)
    {
        setCheckable(true);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);

        if (isChecked() || isDown()) {
            QRect pill(QPoint(0, 0), kPillSize);
            pill.moveCenter(rect().center());
            const int radius = Theme::metric(QStringLiteral("radius-tab"));
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);
            painter.setBrush(Theme::color(isChecked() ? QStringLiteral("tint") : QStringLiteral("track")));
            painter.drawRoundedRect(pill, radius, radius);
        }

        const QPixmap glyph = Theme::icon(mIcon, kIconSize,
                                          isChecked() ? QStringLiteral("primary") : QStringLiteral("text-muted"));
        QRect target(QPoint(0, 0), glyph.size());
        target.moveCenter(rect().center());
        painter.drawPixmap(target, glyph);
    }

private:
    QString mIcon;
};

} // namespace

BottomNavBar::BottomNavBar(QWidget *parent)
    : QWidget(parent)
    , mGroup(new QButtonGroup(this))
{
    setAttribute(Qt::WA_StyledBackground);

    setFixedSize(240, Theme::metric(QStringLiteral("size-nav-height")));

    struct Item { Tab tab; const char *icon; int x; int width; };
    const Item items[] = {
        {Home, "home", 6, 73},
        {Dashboard, "chart", 83, 74},
        {Settings, "settings", 161, 73},
    };

    mGroup->setExclusive(true);
    for (const Item &item : items) {
        auto *button = new NavButton(QLatin1String(item.icon), this);
        button->setGeometry(item.x, 3, item.width, 42);
        mGroup->addButton(button, item.tab);
    }
    mGroup->button(Home)->setChecked(true);

    Language::bind(this, [this]() {
        mGroup->button(Home)->setAccessibleName(tr("Tổng quan"));
        mGroup->button(Dashboard)->setAccessibleName(tr("Biểu đồ"));
        mGroup->button(Settings)->setAccessibleName(tr("Cài đặt"));
    });

    QObject::connect(mGroup, &QButtonGroup::idClicked, this, &BottomNavBar::navRequested);
}

void BottomNavBar::setActive(int index)
{
    QAbstractButton *btn = mGroup->button(index);
    if (btn) {
        btn->setChecked(true);
    }
}
