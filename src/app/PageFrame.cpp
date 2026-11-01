#include "app/PageFrame.h"

#include "app/Theme.h"
#include "widgets/BottomNavBar.h"
#include "widgets/StatusHeader.h"
#include "widgets/SubPage.h"

#include <QStackedWidget>

static constexpr int kScreenWidth = 240;
static constexpr int kScreenHeight = 320;

PageFrame::PageFrame(QWidget *parent)
    : QWidget(parent)
    , mStatusHeader(new StatusHeader(this))
    , mStack(new QStackedWidget(this))
    , mNavBar(new BottomNavBar(this))
{
    setFixedSize(kScreenWidth, kScreenHeight);
    setObjectName(QStringLiteral("AppRoot"));
    setAttribute(Qt::WA_StyledBackground);

    mStatusHeader->setGeometry(0, 0, kScreenWidth, mStatusHeader->height());
    mNavBar->move(0, kScreenHeight - mNavBar->height());

    connect(mNavBar, &BottomNavBar::navRequested, this, &PageFrame::showTab);

    setMode(Mode::Tab);
}

QWidget *PageFrame::currentPage() const
{
    return mStack->currentWidget();
}

void PageFrame::addTab(int tab, QWidget *page)
{
    mTabs.insert(tab, page);
    mStack->addWidget(page);
}

void PageFrame::showTab(int tab)
{
    QWidget *page = mTabs.value(tab);
    if (!page) return;

    mCurrentTab = tab;
    mPageTrail.clear();
    mNavBar->setActive(tab);
    setMode(Mode::Tab);
    showPage(page);
}

void PageFrame::pushPage(SubPage *page)
{
    if (mPageTrail.contains(page)) return;

    if (mStack->indexOf(page) < 0) {
        connect(page, &SubPage::backRequested, this, &PageFrame::popPage);
    }
    mPageTrail.append(page);
    setMode(Mode::SubPage);
    showPage(page);
}

void PageFrame::popPage()
{
    if (mPageTrail.isEmpty()) return;

    mPageTrail.removeLast();
    if (mPageTrail.isEmpty()) {
        showTab(mCurrentTab);
    } else {
        showPage(mPageTrail.last());
    }
}

void PageFrame::showTakeover(QWidget *page)
{
    setMode(Mode::Takeover);
    showPage(page);
}

void PageFrame::showFullScreenPage(QWidget *page)
{
    setMode(Mode::FullScreen);
    showPage(page);
}

void PageFrame::setMode(Mode mode)
{
    mMode = mode;

    const int statusHeight = mStatusHeader->height();
    const int navHeight = Theme::metric(QStringLiteral("size-nav-height"));

    switch (mode) {
    case Mode::Tab:
        mStack->setGeometry(0, statusHeight, kScreenWidth, kScreenHeight - statusHeight - navHeight);
        break;
    case Mode::SubPage:
    case Mode::Takeover:
        mStack->setGeometry(0, statusHeight, kScreenWidth, kScreenHeight - statusHeight);
        break;
    case Mode::FullScreen:
        mStack->setGeometry(0, 0, kScreenWidth, kScreenHeight);
        break;
    }

    mStatusHeader->setVisible(mode != Mode::FullScreen);
    mNavBar->setVisible(mode == Mode::Tab);
    mNavBar->raise();
}

void PageFrame::showPage(QWidget *page)
{
    if (mStack->indexOf(page) < 0) {
        mStack->addWidget(page);
    }
    mStack->setCurrentWidget(page);
}
