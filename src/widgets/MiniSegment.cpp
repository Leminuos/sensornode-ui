#include "widgets/MiniSegment.h"

#include <QButtonGroup>
#include <QPushButton>
#include <QVariant>

MiniSegment::MiniSegment(const QString &left, const QString &right, QWidget *parent)
    : QWidget(parent)
    , mGroup(new QButtonGroup(this))
{
    setFixedSize(86, 40);

    auto *track = new QWidget(this);
    track->setAttribute(Qt::WA_StyledBackground);
    track->setProperty("surface", "miniTrack");
    track->setGeometry(10, 4, 68, 32);

    const struct { const QString &label; const char *side; QRect rect; } cells[] = {
        {left, "left", QRect(0, 0, 44, 40)},
        {right, "right", QRect(44, 0, 42, 40)},
    };
    int id = 0;
    for (const auto &cell : cells) {
        auto *button = new QPushButton(cell.label, this);
        button->setCheckable(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setProperty("miniSegment", cell.side);
        button->setGeometry(cell.rect);
        mGroup->addButton(button, id++);
    }
    mGroup->button(0)->setChecked(true);

    connect(mGroup, &QButtonGroup::idClicked, this, &MiniSegment::currentChanged);
}

int MiniSegment::currentIndex() const
{
    return mGroup->checkedId();
}

void MiniSegment::setCurrentIndex(int index)
{
    if (QAbstractButton *button = mGroup->button(index)) {
        button->setChecked(true);
    }
}
