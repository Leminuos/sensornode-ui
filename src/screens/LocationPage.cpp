#include "screens/LocationPage.h"

#include "app/Theme.h"
#include "model/Locations.h"
#include "widgets/ListRow.h"
#include "widgets/ScrollList.h"

#include <QLabel>

static constexpr int kRowHeight = 44;
static constexpr int kListX = 8;
static constexpr int kListY = 6;
static constexpr int kListWidth = 224;
static constexpr int kListHeight = 244;
static constexpr int kErrorHeight = 20;

LocationPage::LocationPage(QWidget *parent)
    : SubPage(tr("Vị trí"), parent)
    , mList(new ScrollList(content()))
    , mError(new QLabel(tr("Không đổi được vị trí."), content()))
{
    mList->setGeometry(kListX, kListY, kListWidth, kListHeight);

    const QVector<Location> &locations = Locations::all();
    auto *list = new QWidget;
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
    mList->setContent(list);

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
    mList->resize(kListWidth, ok ? kListHeight : kListHeight - kErrorHeight - 2);
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
    mList->scrollToRow(mSelected * kRowHeight, kRowHeight);
}
