#include "screens/UpdatePage.h"

#include "app/Language.h"
#include "app/Theme.h"
#include "widgets/RaisedButton.h"

#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QTimer>
#include <QVariant>

static constexpr int kResultMs = 2500;

UpdatePage::UpdatePage(QWidget *parent)
    : SubPage(QString(), parent)
    , mResultTimer(new QTimer(this))
{
    QWidget *body = content();
    const QMargins shadow = Theme::shadowMargins();

    auto *versionCard = new QFrame(body);
    versionCard->setProperty("surface", "card");
    versionCard->setGeometry(QRect(8, 6, 224, 75).marginsAdded(shadow));

    auto *caption = new QLabel(body);
    caption->setObjectName(QStringLiteral("fieldCaption"));
    caption->setGeometry(18, 16, 204, 16);

    mVersion = new QLabel(QStringLiteral("--"), body);
    mVersion->setObjectName(QStringLiteral("bigValue"));
    mVersion->setGeometry(18, 38, 204, 33);

    auto *notifyCard = new QFrame(body);
    notifyCard->setProperty("surface", "card");
    notifyCard->setGeometry(QRect(8, 88, 224, 66).marginsAdded(shadow));

    auto *title = new QLabel(body);
    title->setObjectName(QStringLiteral("fieldTitle"));
    title->setGeometry(18, 98, 150, 26);

    mToggle = new QCheckBox(body);
    mToggle->setProperty("toggle", true);
    mToggle->setFocusPolicy(Qt::NoFocus);
    mToggle->setGeometry(170, 91, 62, 40);

    auto *note = new QLabel(body);
    note->setObjectName(QStringLiteral("fieldNote"));
    note->setGeometry(18, 130, 204, 14);

    mCheck = new RaisedButton(QString(), RaisedButton::Primary, body);
    mCheck->setVisualGeometry(QRect(8, 208, 224, 42));

    mResultTimer->setSingleShot(true);
    mResultTimer->setInterval(kResultMs);
    connect(mResultTimer, &QTimer::timeout, this, [this]() {
        mResult.clear();
        refreshButton();
    });

    connect(mToggle, &QCheckBox::clicked, this, &UpdatePage::autoNotifyRequested);
    connect(mCheck, &QPushButton::clicked, this, &UpdatePage::checkRequested);

    Language::bind(this, [=]() {
        setTitle(tr("Cập nhật"));
        caption->setText(tr("Phiên bản hiện tại"));
        title->setText(tr("Tự báo bản mới"));
        note->setText(tr("Luôn hỏi trước khi cài."));
        refreshButton();
    });
}

void UpdatePage::setCurrentVersion(const QString &version)
{
    mVersion->setText(version);
}

void UpdatePage::setAutoNotify(bool enabled)
{
    mToggle->setChecked(enabled);
}

void UpdatePage::setAutoNotifyEnabled(bool enabled)
{
    mToggle->setEnabled(enabled);
}

void UpdatePage::setVerifying(bool verifying)
{
    mVerifying = verifying;
    refreshButton();
}

void UpdatePage::onCheckStarted()
{
    mChecking = true;
    mResultTimer->stop();
    mResult.clear();
    refreshButton();
}

void UpdatePage::onUpToDate()
{
    showResult(tr("Đã là bản mới nhất"));
}

void UpdatePage::onCheckFailed()
{
    showResult(tr("Kiểm tra thất bại"));
}

void UpdatePage::onUpdateFound()
{
    mChecking = false;
    refreshButton();
}

void UpdatePage::showResult(const QString &text)
{
    mChecking = false;
    mResult = text;
    mResultTimer->start();
    refreshButton();
}

void UpdatePage::refreshButton()
{
    QString text = tr("Kiểm tra cập nhật");
    if (mVerifying) {
        text = tr("Đang xác minh bản cập nhật");
    } else if (mChecking) {
        text = tr("Đang kiểm tra…");
    } else if (!mResult.isEmpty()) {
        text = mResult;
    }

    mCheck->setText(text);
    mCheck->setEnabled(!mVerifying && !mChecking);
}
