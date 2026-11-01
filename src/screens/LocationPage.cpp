#include "screens/LocationPage.h"

#include "app/Theme.h"
#include "model/Locations.h"
#include "widgets/ListRow.h"

#include <QLabel>
#include <QPainterPath>
#include <QRegion>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QScrollerProperties>

static constexpr int kRowHeight = 44;
static constexpr int kListX = 8;
static constexpr int kListY = 6;
static constexpr int kListWidth = 224;
static constexpr int kListHeight = 244;
static constexpr int kErrorHeight = 20;
static constexpr int kIndicatorWidth = 3;

LocationPage::LocationPage(QWidget *parent)
    : SubPage(tr("Vị trí"), parent)
    , mScroll(new QScrollArea(content()))
    , mIndicator(new QWidget(mScroll))
    , mError(new QLabel(tr("Không đổi được vị trí."), content()))
{
    mScroll->setObjectName(QStringLiteral("locationScroll"));
    mScroll->setGeometry(kListX, kListY, kListWidth, kListHeight);
    mScroll->setFrameShape(QFrame::NoFrame);
    mScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPainterPath corners;
    const int radius = Theme::metric(QStringLiteral("radius-card"));
    corners.addRoundedRect(QRectF(0, 0, kListWidth, kListHeight), radius, radius);
    mScroll->setMask(QRegion(corners.toFillPolygon().toPolygon()));

    const QVector<Location> &locations = Locations::all();
    auto *list = new QWidget;
    list->setObjectName(QStringLiteral("locationList"));
    list->setAttribute(Qt::WA_StyledBackground);
    list->setFixedSize(kListWidth, locations.size() * kRowHeight);

    for (int i = 0; i < locations.size(); ++i) {
        auto *row = new ListRow(locations.at(i).name, list);
        row->setTrailing(ListRow::CheckMark);
        row->setGeometry(0, i * kRowHeight, kListWidth, kRowHeight);
        row->setSeparator(i != 0);
        connect(row, &QAbstractButton::clicked, this, [this, i]() {
            if (i != mSelected) {
                emit locationRequested(i);
            }
        });
        mRows.append(row);
    }
    mScroll->setWidget(list);

    QScroller::grabGesture(mScroll->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(mScroll->viewport());
    QScrollerProperties props = scroller->scrollerProperties();
    props.setScrollMetric(QScrollerProperties::AxisLockThreshold, 1.0);
    props.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.5);
    props.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0);
    props.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0);
    scroller->setScrollerProperties(props);

    mIndicator->setObjectName(QStringLiteral("scrollIndicator"));
    mIndicator->setAttribute(Qt::WA_StyledBackground);
    mIndicator->setAttribute(Qt::WA_TransparentForMouseEvents);
    connect(mScroll->verticalScrollBar(), &QScrollBar::valueChanged, this, &LocationPage::moveIndicator);

    mError->setObjectName(QStringLiteral("inlineError"));
    mError->setGeometry(kListX + 2, kListY + kListHeight - kErrorHeight, kListWidth - 4, kErrorHeight);
    mError->hide();

    setSelectedIndex(0);
    refreshTimes();
}

void LocationPage::setSelectedIndex(int index)
{
    mSelected = index;
    for (int i = 0; i < mRows.size(); ++i) {
        mRows.at(i)->setSelected(i == index);
    }
}

void LocationPage::finishApply(int previousIndex, bool ok)
{
    if (!ok) {
        setSelectedIndex(previousIndex);
    }
    mError->setVisible(!ok);
    mScroll->setFixedHeight(ok ? kListHeight : kListHeight - kErrorHeight - 2);
}

void LocationPage::setClockSynced(bool synced)
{
    mSynced = synced;
    refreshTimes();
}

void LocationPage::refreshTimes()
{
    const QVector<Location> &locations = Locations::all();
    for (int i = 0; i < mRows.size(); ++i) {
        mRows.at(i)->setSubtitle(Locations::offsetText(locations.at(i)));
        mRows.at(i)->setClockText(mSynced ? Locations::timeText(locations.at(i)) : QStringLiteral("--:--"));
    }
}

void LocationPage::showEvent(QShowEvent *event)
{
    SubPage::showEvent(event);
    refreshTimes();
    mScroll->verticalScrollBar()->setValue(qMax(0, mSelected * kRowHeight - (kListHeight - kRowHeight) / 2));
    moveIndicator();
}

void LocationPage::moveIndicator()
{
    const QScrollBar *bar = mScroll->verticalScrollBar();
    const int contentHeight = mScroll->widget()->height();
    const int viewHeight = mScroll->viewport()->height();
    const int track = viewHeight - 8;
    const int thumb = track * viewHeight / contentHeight;
    const int y = 4 + (bar->maximum() > 0 ? (track - thumb) * bar->value() / bar->maximum() : 0);

    mIndicator->setGeometry(kListWidth - kIndicatorWidth - 2, y, kIndicatorWidth, thumb);
    mIndicator->raise();
}
