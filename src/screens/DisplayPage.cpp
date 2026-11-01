#include "screens/DisplayPage.h"

#include "app/Language.h"
#include "app/Theme.h"
#include "widgets/SegmentedControl.h"
#include "widgets/TouchSlider.h"

#include <QFrame>
#include <QLabel>
#include <QVariant>

static const int kAutoDimMinutes[] = {0, 1, 5, 15};

DisplayPage::DisplayPage(QWidget *parent)
    : SubPage(QString(), parent)
{
    QWidget *body = content();

    auto *card = new QFrame(body);
    card->setProperty("surface", "card");
    card->setGeometry(QRect(8, 6, 224, 115).marginsAdded(Theme::shadowMargins()));

    auto *caption = new QLabel(body);
    caption->setObjectName(QStringLiteral("fieldCaption"));
    caption->setGeometry(18, 16, 204, 16);

    mValue = new QLabel(body);
    mValue->setObjectName(QStringLiteral("bigValue"));
    mValue->setGeometry(18, 38, 204, 33);

    mSlider = new TouchSlider(body);
    mSlider->setRange(0, 100);
    mSlider->setGeometry(18, 75, 204, 40);

    auto *section = new QLabel(body);
    section->setObjectName(QStringLiteral("sectionLabel"));
    section->setGeometry(10, 132, 220, 16);

    mAutoDim = new SegmentedControl({QString(), QString(), QString(), QString()}, body);
    mAutoDim->setGeometry(8, 155, 224, 36);

    connect(mSlider, &QSlider::valueChanged, this, [this](int value) {
        mValue->setText(QStringLiteral("%1%").arg(value));
        emit brightnessPreviewed(value);
    });
    connect(mSlider, &QSlider::sliderReleased, this, [this]() {
        emit brightnessCommitted(mSlider->value());
    });
    connect(mAutoDim, &SegmentedControl::currentChanged, this, [this](int index) {
        emit autoDimRequested(kAutoDimMinutes[index]);
    });

    Language::bind(this, [=]() {
        setTitle(tr("Màn hình"));
        caption->setText(tr("Độ sáng"));
        section->setText(tr("Tự giảm sáng khi không dùng"));
        mAutoDim->setLabels({tr("Tắt"), tr("1 phút"), tr("5 phút"), tr("15 phút")});
    });

    setBrightness(0);
}

void DisplayPage::setBrightness(int value)
{
    if (mSlider->isSliderDown()) return;

    const QSignalBlocker blocker(mSlider);
    mSlider->setValue(value);
    mValue->setText(QStringLiteral("%1%").arg(value));
}

void DisplayPage::setBrightnessEnabled(bool enabled)
{
    mSlider->setEnabled(enabled);
}

void DisplayPage::setAutoDimMinutes(int minutes)
{
    for (int i = 0; i < 4; ++i) {
        if (kAutoDimMinutes[i] == minutes) {
            mAutoDim->setCurrentIndex(i);
        }
    }
}
