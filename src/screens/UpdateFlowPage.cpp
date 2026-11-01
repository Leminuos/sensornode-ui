#include "screens/UpdateFlowPage.h"

#include "app/Language.h"
#include "app/Theme.h"
#include "widgets/ProgressRing.h"
#include "widgets/RaisedButton.h"
#include "widgets/StepBar.h"

#include <QLabel>
#include <QPainter>
#include <QVariant>

static constexpr int kWidth = 240;
static constexpr int kInnerX = 10;
static constexpr int kInnerWidth = 220;
static constexpr int kButtonY = 244;
static constexpr int kButtonHeight = 42;
static constexpr int kActionGap = 6;

using Step = StepBar::StepState;

UpdateFlowPage::UpdateFlowPage(QWidget *parent)
    : QWidget(parent)
    , mRing(new ProgressRing(this))
    , mProgressTitle(new QLabel(this))
    , mProgressNote(new QLabel(this))
    , mPowerWarning(new QLabel(this))
    , mHalo(new QLabel(this))
    , mResultTitle(new QLabel(this))
    , mResultBody(new QLabel(this))
    , mSteps(new StepBar({QString(), QString(), QString(), QString()}, this))
    , mPrimary(new RaisedButton(QString(), RaisedButton::Primary, this))
    , mSecondary(new RaisedButton(QString(), RaisedButton::Secondary, this))
{
    mRing->move((kWidth - mRing->width()) / 2, 44);

    mProgressTitle->setObjectName(QStringLiteral("flowTitle"));
    mProgressTitle->setAlignment(Qt::AlignCenter);
    mProgressTitle->setGeometry(0, 168, kWidth, 20);

    mProgressNote->setObjectName(QStringLiteral("flowBody"));
    mProgressNote->setAlignment(Qt::AlignCenter);
    mProgressNote->setGeometry(0, 192, kWidth, 16);

    mPowerWarning->setObjectName(QStringLiteral("powerWarning"));
    mPowerWarning->setAlignment(Qt::AlignCenter);

    Language::bind(this, [this]() {
        mPowerWarning->setText(tr("Không tắt nguồn thiết bị"));
        mPowerWarning->ensurePolished();
        const QSize warning = mPowerWarning->sizeHint();
        mPowerWarning->setGeometry((kWidth - warning.width()) / 2, 192, warning.width(), warning.height());
        mSteps->setLabels({tr("Tải về"), tr("Cài đặt"), tr("Khởi động"), tr("Kiểm tra")});
    });

    mHalo->setGeometry((kWidth - 76) / 2, 8, 76, 75);

    mResultTitle->setObjectName(QStringLiteral("flowTitle"));
    mResultTitle->setAlignment(Qt::AlignCenter);
    mResultTitle->setGeometry(0, 84, kWidth, 20);

    mResultBody->setObjectName(QStringLiteral("flowBody"));
    mResultBody->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    mResultBody->setWordWrap(true);
    mResultBody->setGeometry(kInnerX + 6, 112, kInnerWidth - 12, 48);

    connect(mPrimary, &QPushButton::clicked, this, [this]() {
        if (mView == View::Installed) {
            emit rebootRequested();
        } else {
            emit retryRequested();
        }
    });
    connect(mSecondary, &QPushButton::clicked, this, [this]() {
        if (mView == View::RolledBack) {
            emit rollbackAcknowledged();
        } else {
            emit closeRequested();
        }
    });
}

void UpdateFlowPage::setProgress(int percent)
{
    mRing->setPercent(percent);
}

void UpdateFlowPage::showProgressLayout(bool warning)
{
    mRing->show();
    mProgressTitle->show();
    mProgressNote->setVisible(!warning);
    mPowerWarning->setVisible(warning);
    mHalo->hide();
    mResultTitle->hide();
    mResultBody->hide();

    mSteps->setGeometry(4, 247, 232, mSteps->height());
    setActions(Actions::None);
}

void UpdateFlowPage::showResultLayout(bool danger, const QString &glyph, const QString &title, const QString &body,
                                      bool bodyIsError)
{
    mRing->hide();
    mProgressTitle->hide();
    mProgressNote->hide();
    mPowerWarning->hide();
    mHalo->show();
    mResultTitle->show();
    mResultBody->show();

    QPixmap halo(danger ? QStringLiteral(":/surfaces/halo_danger.png") : QStringLiteral(":/surfaces/halo_primary.png"));
    {
        QPainter painter(&halo);
        painter.drawPixmap(21, 19, Theme::icon(glyph, 34, danger ? QStringLiteral("on-danger") : QStringLiteral("on-primary")));
    }
    mHalo->setPixmap(halo);

    mResultTitle->setText(title);
    mResultBody->setText(body);
    mResultBody->setProperty("tone", bodyIsError ? "bad" : "normal");
    Theme::repolish(mResultBody);

    mSteps->setGeometry(2, 199, 236, mSteps->height());
}

void UpdateFlowPage::setActions(Actions actions)
{
    const QRect full(kInnerX, kButtonY, kInnerWidth, kButtonHeight);
    const int half = (kInnerWidth - kActionGap) / 2;

    const bool split = actions == Actions::CloseRetry;
    for (RaisedButton *button : {mPrimary, mSecondary}) {
        button->setProperty("compact", split);
        Theme::repolish(button);
    }

    mPrimary->setVisible(actions == Actions::Reboot || actions == Actions::RebootLocked || split);
    mSecondary->setVisible(actions == Actions::Acknowledge || split);
    mPrimary->setEnabled(actions != Actions::RebootLocked);

    switch (actions) {
    case Actions::None:
        break;
    case Actions::Reboot:
        mPrimary->setText(tr("Khởi động lại"));
        mPrimary->setVisualGeometry(full);
        break;
    case Actions::RebootLocked:
        mPrimary->setText(tr("Khởi động lại…"));
        mPrimary->setVisualGeometry(full);
        break;
    case Actions::CloseRetry:
        mSecondary->setText(tr("Đóng"));
        mSecondary->setVisualGeometry(QRect(kInnerX, kButtonY, half, kButtonHeight));
        mPrimary->setText(tr("Thử lại"));
        mPrimary->setVisualGeometry(QRect(kInnerX + half + kActionGap, kButtonY, half, kButtonHeight));
        break;
    case Actions::Acknowledge:
        mSecondary->setText(tr("Đã hiểu"));
        mSecondary->setVisualGeometry(full);
        break;
    }
}

void UpdateFlowPage::showDownloading(const QString &version, int sizeMb)
{
    mView = View::Downloading;
    showProgressLayout(false);
    mRing->setPercent(0);
    mProgressTitle->setText(tr("Đang tải về"));
    mProgressNote->setText(sizeMb > 0 ? tr("Bản %1, khoảng %2 MB").arg(version).arg(sizeMb) : tr("Bản %1").arg(version));
    mSteps->setStates({Step::Active, Step::Pending, Step::Pending, Step::Pending});
}

void UpdateFlowPage::showInstalling()
{
    mView = View::Installing;
    showProgressLayout(true);
    mProgressTitle->setText(tr("Đang cài đặt"));
    mSteps->setStates({Step::Done, Step::Active, Step::Pending, Step::Pending});
}

void UpdateFlowPage::showInstalled(const QString &version, bool rebootFailed)
{
    mView = View::Installed;
    const QString title = version.isEmpty() ? tr("Đã cài xong") : tr("Đã cài xong %1").arg(version);
    showResultLayout(false, QStringLiteral("check"), title,
                     rebootFailed ? tr("Không khởi động lại được.") : tr("Khởi động lại để dùng bản mới."),
                     rebootFailed);
    mSteps->setStates({Step::Done, Step::Done, Step::Active, Step::Pending});
    setActions(Actions::Reboot);
}

void UpdateFlowPage::showRebooting()
{
    mView = View::Rebooting;
    showResultLayout(false, QStringLiteral("clock"), tr("Đang khởi động lại"),
                     tr("Màn hình sẽ tắt một lúc. Thiết bị tự bật lại."));
    mSteps->setStates({Step::Done, Step::Done, Step::Active, Step::Pending});
    setActions(Actions::RebootLocked);
}

void UpdateFlowPage::showDownloadFailed()
{
    mView = View::DownloadFailed;
    showResultLayout(true, QStringLiteral("close"), tr("Tải về thất bại"),
                     tr("Không tải được bản cập nhật. Kiểm tra mạng rồi thử lại."));
    mSteps->setStates({Step::Failed, Step::Pending, Step::Pending, Step::Pending});
    setActions(Actions::CloseRetry);
}

void UpdateFlowPage::showInstallFailed(const QString &currentVersion)
{
    mView = View::InstallFailed;
    showResultLayout(true, QStringLiteral("close"), tr("Cài đặt thất bại"),
                     tr("Thiết bị vẫn chạy bản %1 và hoạt động bình thường.").arg(currentVersion));
    mSteps->setStates({Step::Done, Step::Failed, Step::Pending, Step::Pending});
    setActions(Actions::CloseRetry);
}

void UpdateFlowPage::showRolledBack(const QString &currentVersion, const QString &failedVersion)
{
    mView = View::RolledBack;
    showResultLayout(true, QStringLiteral("close"), tr("Đã quay lại bản %1").arg(currentVersion),
                     tr("Bản %1 không chạy ổn định nên thiết bị tự về bản cũ.").arg(failedVersion));
    mSteps->setStates({Step::Done, Step::Done, Step::Done, Step::Failed});
    setActions(Actions::Acknowledge);
}
