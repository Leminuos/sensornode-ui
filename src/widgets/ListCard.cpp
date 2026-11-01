#include "widgets/ListCard.h"

#include "app/Theme.h"
#include "widgets/ListRow.h"

#include <QVariant>

static constexpr int kCardWidth = 224;
static constexpr int kRowHeight = 44;

ListCard::ListCard(QWidget *parent)
    : QFrame(parent)
{
    setProperty("surface", "card");
}

ListRow *ListCard::addRow(const QString &glyph, const QString &tile)
{
    auto *row = new ListRow(QString(), this);
    row->setLeadingIcon(glyph, tile);
    mRows.append(row);
    layoutRows();
    return row;
}

void ListCard::setRowHidden(ListRow *row, bool hidden)
{
    row->setHidden(hidden);
    layoutRows();
}

void ListCard::place(int x, int y)
{
    mOrigin = QPoint(x, y);
    layoutRows();
}

void ListCard::layoutRows()
{
    const QMargins shadow = Theme::shadowMargins();

    int visible = 0;
    for (ListRow *row : qAsConst(mRows)) {
        if (row->isHidden()) continue;
        row->setGeometry(shadow.left(), shadow.top() + visible * kRowHeight, kCardWidth, kRowHeight);
        row->setSeparator(visible > 0);
        ++visible;
    }
    setGeometry(QRect(mOrigin, QSize(kCardWidth, visible * kRowHeight)).marginsAdded(shadow));
}
