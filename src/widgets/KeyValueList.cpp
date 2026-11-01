#include "widgets/KeyValueList.h"

#include "app/Theme.h"

#include <QLabel>
#include <QProgressBar>
#include <QVariant>

static constexpr int kCardWidth = 224;
static constexpr int kPadX = 10;
static constexpr int kRowHeight = 27;
static constexpr int kBarRowHeight = 35;
static constexpr int kBarHeight = 6;

KeyValueList::KeyValueList(QWidget *parent)
    : QFrame(parent)
    , mBottom(Theme::shadowMargins().top())
{
    setProperty("surface", "card");
}

KeyValueList::Row KeyValueList::makeRow(const QString &label)
{
    const QMargins shadow = Theme::shadowMargins();

    Row row;
    row.bar = nullptr;
    row.label = new QLabel(label, this);
    row.label->setObjectName(QStringLiteral("kvLabel"));
    row.label->setGeometry(shadow.left() + kPadX, mBottom, 120, kRowHeight);

    row.value = new QLabel(QStringLiteral("--"), this);
    row.value->setObjectName(QStringLiteral("kvValue"));
    row.value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    row.value->setGeometry(shadow.left() + kPadX, mBottom, kCardWidth - 2 * kPadX, kRowHeight);
    return row;
}

int KeyValueList::addRow(const QString &label)
{
    mRows.append(makeRow(label));
    mBottom += kRowHeight;
    return mRows.size() - 1;
}

int KeyValueList::addChipRow(const QString &label)
{
    Row row = makeRow(label);
    row.value->setObjectName(QString());
    row.value->setProperty("chip", "neutral");
    mRows.append(row);
    mBottom += kRowHeight;
    return mRows.size() - 1;
}

int KeyValueList::addBarRow(const QString &label)
{
    Row row = makeRow(label);
    row.label->setFixedHeight(21);
    row.value->setFixedHeight(21);

    row.bar = new QProgressBar(this);
    row.bar->setObjectName(QStringLiteral("usageBar"));
    row.bar->setTextVisible(false);
    row.bar->setRange(0, 100);
    row.bar->setGeometry(Theme::shadowMargins().left() + kPadX, mBottom + 24, kCardWidth - 2 * kPadX, kBarHeight);

    mRows.append(row);
    mBottom += kBarRowHeight;
    return mRows.size() - 1;
}

void KeyValueList::place(int x, int y)
{
    const QMargins shadow = Theme::shadowMargins();
    setGeometry(QRect(x, y, kCardWidth, mBottom - shadow.top()).marginsAdded(shadow));
}

void KeyValueList::setValue(int row, const QString &text, Tone tone)
{
    QLabel *value = mRows.at(row).value;
    value->setText(text);
    value->setProperty("tone", tone == Good ? "good" : tone == Bad ? "bad" : "normal");
    Theme::repolish(value);
}

void KeyValueList::setChip(int row, const QString &text, const char *kind)
{
    QLabel *chip = mRows.at(row).value;
    chip->setText(text);
    chip->setProperty("chip", kind);
    Theme::repolish(chip);

    chip->ensurePolished();
    const QSize hint = chip->sizeHint();
    const Row &entry = mRows.at(row);
    const QRect slot = entry.label->geometry();
    chip->setAlignment(Qt::AlignCenter);
    chip->setGeometry(Theme::shadowMargins().left() + kCardWidth - kPadX - hint.width(),
                      slot.top() + (kRowHeight - hint.height()) / 2, hint.width(), hint.height());
}

void KeyValueList::setBar(int row, int percent)
{
    mRows.at(row).bar->setValue(qBound(0, percent, 100));
}

QString KeyValueList::value(int row) const
{
    return mRows.at(row).value->text();
}

QString KeyValueList::label(int row) const
{
    return mRows.at(row).label->text();
}
