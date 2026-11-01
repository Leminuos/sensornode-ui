#include "screens/SamplingPage.h"

#include "app/Theme.h"
#include "widgets/ListRow.h"

#include <QFrame>
#include <QLabel>
#include <QVariant>

static constexpr int kRowHeight = 44;
static const int kPeriods[] = {5, 10, 30, 60};

QString SamplingPage::periodText(int seconds)
{
    if (seconds > 0 && seconds % 60 == 0) {
        return tr("%1 phút").arg(seconds / 60);
    }
    return tr("%1 giây").arg(seconds);
}

SamplingPage::SamplingPage(QWidget *parent)
    : SubPage(tr("Chu kỳ lấy mẫu"), parent)
{
    QWidget *body = content();

    auto *card = new QFrame(body);
    card->setProperty("surface", "card");
    card->setGeometry(QRect(8, 6, 224, 4 * kRowHeight).marginsAdded(Theme::shadowMargins()));

    int y = 6;
    for (int seconds : kPeriods) {
        auto *row = new ListRow(periodText(seconds), body);
        row->setTrailing(ListRow::CheckMark);
        row->setGeometry(8, y, 224, kRowHeight);
        row->setSeparator(y != 6);
        connect(row, &QAbstractButton::clicked, this, [this, seconds]() {
            if (seconds != mPeriodSec && mPendingSec == 0) {
                emit periodRequested(seconds);
            }
        });
        mRows.append(row);
        y += kRowHeight;
    }

    mError = new QLabel(tr("Không đổi được chu kỳ. Dịch vụ đo không phản hồi."), body);
    mError->setObjectName(QStringLiteral("inlineError"));
    mError->setWordWrap(true);
    mError->setGeometry(10, y + 7, 220, 34);
    mError->hide();

    refreshRows();
}

void SamplingPage::setPeriod(int seconds)
{
    mPeriodSec = seconds;
    refreshRows();
}

void SamplingPage::beginApply(int seconds)
{
    mPendingSec = seconds;
    mError->hide();
    refreshRows();
}

void SamplingPage::finishApply(bool ok)
{
    if (ok) {
        mPeriodSec = mPendingSec;
    }
    mPendingSec = 0;
    mError->setVisible(!ok);
    refreshRows();
}

void SamplingPage::refreshRows()
{
    const int active = mPendingSec != 0 ? mPendingSec : mPeriodSec;

    for (int i = 0; i < mRows.size(); ++i) {
        ListRow *row = mRows.at(i);
        const int seconds = kPeriods[i];
        row->setSelected(seconds == active);
        row->setEnabled(mPendingSec == 0 || seconds == mPendingSec);

        if (seconds == mPendingSec) {
            row->setValue(tr("Đang áp dụng…"));
        } else if (seconds == kPeriods[0]) {
            row->setValue(tr("Mặc định"));
        } else {
            row->setValue(QString());
        }
    }
}
