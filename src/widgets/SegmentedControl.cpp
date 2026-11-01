#include "widgets/SegmentedControl.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVariant>

static constexpr int kHeight = 36;

SegmentedControl::SegmentedControl(const QStringList &labels, QWidget *parent)
    : QWidget(parent)
    , mGroup(new QButtonGroup(this))
{
    setAttribute(Qt::WA_StyledBackground);
    setProperty("surface", "segTrack");
    setFixedHeight(kHeight);

    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(2, 0, 2, 0);
    row->setSpacing(0);

    for (int i = 0; i < labels.size(); ++i) {
        auto *button = new QPushButton(labels.at(i), this);
        button->setCheckable(true);
        button->setProperty("segment", true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
        mGroup->addButton(button, i);
        row->addWidget(button, 1);
    }

    if (!labels.isEmpty()) {
        mGroup->button(0)->setChecked(true);
    }

    connect(mGroup, &QButtonGroup::idClicked, this, &SegmentedControl::currentChanged);
}

void SegmentedControl::setLabels(const QStringList &labels)
{
    for (int i = 0; i < labels.size(); ++i) {
        mGroup->button(i)->setText(labels.at(i));
    }
}

int SegmentedControl::currentIndex() const
{
    return mGroup->checkedId();
}

void SegmentedControl::setCurrentIndex(int index)
{
    if (QAbstractButton *button = mGroup->button(index)) {
        button->setChecked(true);
    }
}
