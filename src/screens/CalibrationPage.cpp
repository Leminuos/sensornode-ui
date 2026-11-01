#include "screens/CalibrationPage.h"

#include "app/Language.h"
#include "app/Theme.h"
#include "widgets/RaisedButton.h"

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QVariant>

static constexpr int kCrossSize = 24;
static constexpr int kCrossThickness = 2;
static constexpr int kDotSize = 8;
static constexpr int kPromptY = 138;
static constexpr int kIdleTimeoutMs = 30000;
static constexpr int kVerifySeconds = 15;
static constexpr int kInnerX = 10;
static constexpr int kInnerWidth = 220;

const QVector<QPoint> &CalibrationPage::targets()
{
    static const QVector<QPoint> points = {{24, 24}, {216, 24}, {216, 296}, {24, 296}, {120, 160}};
    return points;
}

CalibrationPage::CalibrationPage(QWidget *parent)
    : QWidget(parent)
    , mQuestion(new QLabel(this))
    , mKeep(new RaisedButton(QString(), RaisedButton::Primary, this))
    , mCountdownLabel(new QLabel(this))
    , mAgain(new RaisedButton(QString(), RaisedButton::Secondary, this))
{
    setAttribute(Qt::WA_StyledBackground);

    mQuestion->setObjectName(QStringLiteral("calibrationQuestion"));
    mQuestion->setAlignment(Qt::AlignCenter);
    mQuestion->setGeometry(kInnerX, 16, kInnerWidth, 17);

    mKeep->setProperty("big", true);
    mKeep->setVisualGeometry(QRect(kInnerX, 43, kInnerWidth, 92));

    mCountdownLabel->setObjectName(QStringLiteral("calibrationCountdown"));
    mCountdownLabel->setAlignment(Qt::AlignCenter);
    mCountdownLabel->setGeometry(kInnerX, 145, kInnerWidth, 20);

    mAgain->setVisualGeometry(QRect(kInnerX, 262, kInnerWidth, 42));

    showVerifyWidgets(false);

    mIdleTimer.setSingleShot(true);
    mIdleTimer.setInterval(kIdleTimeoutMs);
    connect(&mIdleTimer, &QTimer::timeout, this, [this]() {
        mStep = Step::Idle;
        emit cancelled();
    });

    mCountdown.setInterval(1000);
    connect(&mCountdown, &QTimer::timeout, this, [this]() {
        --mSecondsLeft;
        updateCountdown();
        if (mSecondsLeft <= 0) {
            mCountdown.stop();
            mStep = Step::Idle;
            emit reverted();
        }
    });

    connect(mKeep, &QPushButton::clicked, this, [this]() {
        mCountdown.stop();
        mStep = Step::Idle;
        emit kept();
    });
    connect(mAgain, &QPushButton::clicked, this, [this]() {
        mCountdown.stop();
        emit restartRequested();
    });

    Language::bind(this, [this]() {
        mQuestion->setText(tr("Chạm có trúng không?"));
        mKeep->setText(tr("Giữ hiệu chỉnh này"));
        mAgain->setText(tr("Hiệu chỉnh lại"));
        updateCountdown();
        update();
    });
}

void CalibrationPage::beginPoints()
{
    mCountdown.stop();
    mTouched.clear();
    mStep = Step::Points;
    showVerifyWidgets(false);
    mIdleTimer.start();
    update();
}

void CalibrationPage::beginVerify()
{
    mIdleTimer.stop();
    mStep = Step::Verify;
    mSecondsLeft = kVerifySeconds;
    updateCountdown();
    showVerifyWidgets(true);
    mCountdown.start();
    update();
}

void CalibrationPage::showVerifyWidgets(bool visible)
{
    mQuestion->setVisible(visible);
    mKeep->setVisible(visible);
    mCountdownLabel->setVisible(visible);
    mAgain->setVisible(visible);
}

void CalibrationPage::updateCountdown()
{
    const QString number = QStringLiteral("<b style=\"color:%1;font-size:%2px\">%3</b>")
                               .arg(Theme::color(QStringLiteral("calib-count")).name())
                               .arg(Theme::metric(QStringLiteral("font-title")))
                               .arg(mSecondsLeft);
    mCountdownLabel->setText(tr("Tự quay lại bản cũ sau %1 giây").arg(number));
}

void CalibrationPage::mousePressEvent(QMouseEvent *event)
{
    if (mStep != Step::Points) return;

    mTouched.append(event->localPos());
    update();

    if (mTouched.size() == targets().size()) {
        mIdleTimer.stop();
        emit pointsCaptured(mTouched);
    } else {
        mIdleTimer.start();
    }
}

void CalibrationPage::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if (mStep != Step::Points) return;

    QPainter painter(this);

    for (int i = 0; i < mTouched.size(); ++i) {
        const QPoint target = targets().at(i);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Theme::color(QStringLiteral("calib-point")));
        painter.drawEllipse(target, kDotSize / 2, kDotSize / 2);
    }

    const int point = mTouched.size();
    if (point < targets().size()) {
        const QPoint target = targets().at(point);
        const QColor color = Theme::color(QStringLiteral("calib-fg"));
        painter.fillRect(QRect(target.x() - kCrossSize / 2, target.y() - kCrossThickness / 2, kCrossSize, kCrossThickness), color);
        painter.fillRect(QRect(target.x() - kCrossThickness / 2, target.y() - kCrossSize / 2, kCrossThickness, kCrossSize), color);
    }

    painter.setPen(Theme::color(QStringLiteral("calib-fg")));
    painter.setFont(Theme::font(QStringLiteral("unit"), true));
    painter.drawText(QRect(0, kPromptY, width(), 21), Qt::AlignCenter,
                     tr("Điểm %1/%2").arg(qMin(point + 1, targets().size())).arg(targets().size()));

    painter.setPen(Theme::color(QStringLiteral("calib-muted")));
    painter.setFont(Theme::font(QStringLiteral("small")));
    painter.drawText(QRect(0, kPromptY + 21, width(), 16), Qt::AlignCenter, tr("Chạm vào dấu chữ thập"));
}
