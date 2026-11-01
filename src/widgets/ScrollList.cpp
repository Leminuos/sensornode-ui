#include "widgets/ScrollList.h"

#include "app/Theme.h"

#include <QPainterPath>
#include <QRegion>
#include <QResizeEvent>
#include <QScrollBar>
#include <QScroller>
#include <QScrollerProperties>

static constexpr int kIndicatorWidth = 3;

ScrollList::ScrollList(QWidget *parent)
    : QScrollArea(parent)
    , mIndicator(new QWidget(this))
{
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(viewport());
    QScrollerProperties props = scroller->scrollerProperties();
    props.setScrollMetric(QScrollerProperties::AxisLockThreshold, 1.0);
    props.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.5);
    props.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0);
    props.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0);
    scroller->setScrollerProperties(props);

    mIndicator->setObjectName(QStringLiteral("scrollIndicator"));
    mIndicator->setAttribute(Qt::WA_StyledBackground);
    mIndicator->setAttribute(Qt::WA_TransparentForMouseEvents);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &ScrollList::moveIndicator);
    connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, &ScrollList::moveIndicator);
}

void ScrollList::setViewGeometry(const QRect &rect)
{
    setGeometry(rect);
}

void ScrollList::setContent(QWidget *content)
{
    content->setObjectName(QStringLiteral("scrollContent"));
    content->setAttribute(Qt::WA_StyledBackground);
    delete takeWidget();
    setWidget(content);
    moveIndicator();
}

void ScrollList::scrollToRow(int rowTop, int rowHeight)
{
    verticalScrollBar()->setValue(qMax(0, rowTop - (viewport()->height() - rowHeight) / 2));
    moveIndicator();
}

void ScrollList::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);

    QPainterPath corners;
    const int radius = Theme::metric(QStringLiteral("radius-card"));
    corners.addRoundedRect(QRectF(rect()), radius, radius);
    setMask(QRegion(corners.toFillPolygon().toPolygon()));

    moveIndicator();
}

void ScrollList::moveIndicator()
{
    const QScrollBar *bar = verticalScrollBar();
    if (!widget() || bar->maximum() <= 0) {
        mIndicator->hide();
        return;
    }

    const int view = viewport()->height();
    const int track = view - 8;
    const int thumb = track * view / widget()->height();
    const int y = 4 + (track - thumb) * bar->value() / bar->maximum();

    mIndicator->setGeometry(width() - kIndicatorWidth - 2, y, kIndicatorWidth, thumb);
    mIndicator->show();
    mIndicator->raise();
}
