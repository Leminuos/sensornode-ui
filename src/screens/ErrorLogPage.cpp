#include "screens/ErrorLogPage.h"

#include "app/Theme.h"
#include "widgets/ScrollList.h"

#include <QLabel>
#include <QPainter>
#include <QTextLayout>
#include <QVariant>

static constexpr int kPageX = 8;
static constexpr int kPageY = 6;
static constexpr int kPageWidth = 224;
static constexpr int kMaxHeight = 244;
static constexpr int kPadX = 10;
static constexpr int kPadY = 7;
static constexpr int kMaxLines = 2;

namespace {

class LogRow : public QWidget
{
public:
    LogRow(const QString &caption, const QString &message, bool separator, QWidget *parent)
        : QWidget(parent)
        , mCaption(caption)
        , mSeparator(separator)
    {
        const QFontMetrics metrics(Theme::font(QStringLiteral("small")));
        const int textWidth = kPageWidth - 2 * kPadX;

        QTextLayout layout(message, Theme::font(QStringLiteral("small")));
        layout.beginLayout();
        int consumed = 0;
        while (mLines.size() < kMaxLines) {
            QTextLine line = layout.createLine();
            if (!line.isValid()) break;
            line.setLineWidth(textWidth);
            const bool last = mLines.size() == kMaxLines - 1;
            const QString rest = message.mid(consumed);
            mLines.append(last ? metrics.elidedText(rest, Qt::ElideRight, textWidth)
                               : message.mid(line.textStart(), line.textLength()));
            consumed = line.textStart() + line.textLength();
        }
        layout.endLayout();

        setFixedSize(kPageWidth, 2 * kPadY + captionHeight() + mLines.size() * metrics.height());
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);

        if (mSeparator) {
            painter.fillRect(QRect(kPadX, 0, width() - 2 * kPadX, 1), Theme::color(QStringLiteral("divider")));
        }

        painter.setFont(Theme::font(QStringLiteral("caption")));
        painter.setPen(Theme::color(QStringLiteral("text-muted")));
        painter.drawText(QRect(kPadX, kPadY, width() - 2 * kPadX, captionHeight()), Qt::AlignVCenter, mCaption);

        painter.setFont(Theme::font(QStringLiteral("small")));
        painter.setPen(Theme::color(QStringLiteral("text")));
        int y = kPadY + captionHeight();
        const int lineHeight = painter.fontMetrics().height();
        for (const QString &line : qAsConst(mLines)) {
            painter.drawText(QRect(kPadX, y, width() - 2 * kPadX, lineHeight), Qt::AlignVCenter, line);
            y += lineHeight;
        }
    }

private:
    static int captionHeight()
    {
        return QFontMetrics(Theme::font(QStringLiteral("caption"))).height();
    }

    QString mCaption;
    QStringList mLines;
    bool mSeparator;
};

} // namespace

ErrorLogPage::ErrorLogPage(QWidget *parent)
    : SubPage(tr("Nhật ký lỗi"), parent)
    , mCard(new QFrame(content()))
    , mList(new ScrollList(content()))
    , mMessage(new QLabel(content()))
{
    mCard->setProperty("surface", "card");
    mCard->hide();
    mList->hide();

    mMessage->setObjectName(QStringLiteral("logEmpty"));
    mMessage->setAlignment(Qt::AlignCenter);
    mMessage->setGeometry(kPageX, kPageY, kPageWidth, kMaxHeight);
}

QString ErrorLogPage::timeLabel(const QDateTime &time, const QDateTime &now)
{
    const qint64 days = time.date().daysTo(now.date());
    if (days == 0) return time.toString(QStringLiteral("HH:mm:ss"));
    if (days == 1) return tr("Hôm qua %1").arg(time.toString(QStringLiteral("HH:mm")));
    return time.toString(QStringLiteral("dd/MM HH:mm"));
}

void ErrorLogPage::setEntries(const QVector<LogEntry> &entries)
{
    if (entries.isEmpty()) {
        mCard->hide();
        mList->hide();
        mMessage->setText(tr("Chưa ghi nhận lỗi"));
        mMessage->show();
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();

    auto *list = new QWidget;
    int y = 0;
    for (int i = 0; i < entries.size(); ++i) {
        const LogEntry &entry = entries.at(i);
        const QString caption = QStringLiteral("%1 · %2").arg(timeLabel(entry.time, now), entry.source);
        auto *row = new LogRow(caption, entry.message, i != 0, list);
        row->move(0, y);
        y += row->height();
    }
    list->setFixedSize(kPageWidth, y);
    mList->setContent(list);

    const QRect view(kPageX, kPageY, kPageWidth, qMin(y, kMaxHeight));
    mList->setGeometry(view);
    mCard->setGeometry(view.marginsAdded(Theme::shadowMargins()));
    mCard->show();
    mList->show();
    mMessage->hide();
}

void ErrorLogPage::showLoadFailed()
{
    mCard->hide();
    mList->hide();
    mMessage->setText(tr("Không đọc được nhật ký."));
    mMessage->show();
}

void ErrorLogPage::showEvent(QShowEvent *event)
{
    SubPage::showEvent(event);
    emit refreshRequested();
}
