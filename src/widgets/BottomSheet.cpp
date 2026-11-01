#include "widgets/BottomSheet.h"

#include "app/Theme.h"

#include <QLabel>
#include <QVariant>

static constexpr int kPanelShadow = 12;
static constexpr int kPadX = 12;
static constexpr int kPadTop = 14;
static constexpr int kPadBottom = 12;
static constexpr int kGap = 10;
static constexpr int kActionGap = 6;
static constexpr int kButtonHeight = 42;
static constexpr int kInnerWidth = 240 - 2 * kPadX;

BottomSheet::BottomSheet(QWidget *parent)
    : QWidget(parent)
    , mPanel(new QWidget(this))
    , mTitle(new QLabel(mPanel))
    , mBody(new QLabel(mPanel))
    , mReject(new RaisedButton(QString(), RaisedButton::Secondary, mPanel))
    , mAccept(new RaisedButton(QString(), RaisedButton::Primary, mPanel))
{
    setAttribute(Qt::WA_StyledBackground);
    hide();

    mPanel->setObjectName(QStringLiteral("sheetPanel"));
    mPanel->setAttribute(Qt::WA_StyledBackground);

    mTitle->setObjectName(QStringLiteral("sheetTitle"));
    mBody->setObjectName(QStringLiteral("sheetBody"));
    mBody->setWordWrap(true);

    connect(mReject, &QPushButton::clicked, this, [this]() {
        hide();
        emit rejected();
    });
    connect(mAccept, &QPushButton::clicked, this, [this]() {
        hide();
        emit accepted();
    });
}

void BottomSheet::setTitle(const QString &title)
{
    mTitle->setText(title);
}

void BottomSheet::setBody(const QString &body)
{
    mBody->setText(body);
}

void BottomSheet::setActions(const QString &rejectText, const QString &acceptText, RaisedButton::Variant acceptVariant)
{
    mReject->setText(rejectText);
    mAccept->setText(acceptText);
    mAccept->setProperty("variant", acceptVariant == RaisedButton::Danger    ? "danger"
                                    : acceptVariant == RaisedButton::Primary ? "primary"
                                                                             : "secondary");
    Theme::repolish(mAccept);
}

void BottomSheet::present()
{
    const int statusHeight = Theme::metric(QStringLiteral("size-status-height"));
    setGeometry(0, statusHeight, 240, parentWidget()->height() - statusHeight);

    mTitle->ensurePolished();
    mBody->ensurePolished();
    const int titleHeight = mTitle->sizeHint().height();
    const int bodyHeight = mBody->heightForWidth(kInnerWidth);

    int y = kPanelShadow + kPadTop;
    mTitle->setGeometry(kPadX, y, kInnerWidth, titleHeight);
    y += titleHeight + kGap;
    mBody->setGeometry(kPadX, y, kInnerWidth, bodyHeight);
    y += bodyHeight + kGap;

    const int buttonWidth = (kInnerWidth - kActionGap) / 2;
    mReject->setVisualGeometry(QRect(kPadX, y, buttonWidth, kButtonHeight));
    mAccept->setVisualGeometry(QRect(kPadX + buttonWidth + kActionGap, y, buttonWidth, kButtonHeight));
    y += kButtonHeight + kPadBottom;

    mPanel->setGeometry(0, height() - y, 240, y);

    raise();
    show();
}
