#include "widgets/SubPage.h"

#include "app/Language.h"
#include "app/Theme.h"

#include <QIcon>
#include <QLabel>
#include <QPushButton>

static constexpr int kBackIconSize = 18;
static constexpr int kTitleX = 46;

SubPage::SubPage(const QString &title, QWidget *parent)
    : QWidget(parent)
    , mTitle(new QLabel(title, this))
    , mContent(new QWidget(this))
{
    const int barHeight = Theme::metric(QStringLiteral("size-title-height"));

    mTitle->setObjectName(QStringLiteral("subPageTitle"));
    mTitle->setGeometry(kTitleX, 0, 240 - kTitleX - Theme::metric(QStringLiteral("space-gutter")), barHeight);

    mContent->setGeometry(0, barHeight, 240, 256);

    // Vùng chạm 44×40 lấn 2 px xuống phần đệm của nội dung vì thanh tiêu đề chỉ cao 38.
    auto *back = new QPushButton(this);
    back->setObjectName(QStringLiteral("backButton"));
    back->setGeometry(0, 0, 44, 40);
    back->setFocusPolicy(Qt::NoFocus);
    back->setIcon(Theme::icon(QStringLiteral("back"), kBackIconSize, QStringLiteral("text")));
    back->setIconSize(QSize(kBackIconSize, kBackIconSize));
    Language::bind(back, [back]() { back->setAccessibleName(tr("Quay lại")); });
    connect(back, &QPushButton::clicked, this, &SubPage::backRequested);
}

void SubPage::setTitle(const QString &title)
{
    mTitle->setText(title);
}
