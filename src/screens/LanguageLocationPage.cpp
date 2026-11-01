#include "screens/LanguageLocationPage.h"

#include "app/Language.h"
#include "app/Theme.h"
#include "widgets/ListRow.h"
#include "widgets/SegmentedControl.h"

#include <QFrame>
#include <QLabel>
#include <QVariant>

static constexpr int kRowHeight = 44;

LanguageLocationPage::LanguageLocationPage(QWidget *parent)
    : SubPage(QString(), parent)
{
    QWidget *body = content();

    auto *section = new QLabel(body);
    section->setObjectName(QStringLiteral("sectionLabel"));
    section->setGeometry(10, 10, 220, 16);

    mLanguage = new SegmentedControl({QStringLiteral("Tiếng Việt"), QStringLiteral("English")}, body);
    mLanguage->setGeometry(8, 33, 224, 36);

    auto *card = new QFrame(body);
    card->setProperty("surface", "card");
    card->setGeometry(QRect(8, 76, 224, kRowHeight).marginsAdded(Theme::shadowMargins()));

    mLocation = new ListRow(QString(), body);
    mLocation->setLeadingIcon(QStringLiteral("pin"), QStringLiteral("red"));
    mLocation->setSeparator(false);
    mLocation->setGeometry(8, 76, 224, kRowHeight);

    Language::bind(this, [=]() {
        setTitle(tr("Ngôn ngữ & vị trí"));
        section->setText(tr("Ngôn ngữ"));
        mLocation->setTitle(tr("Vị trí"));
    });

    connect(mLanguage, &SegmentedControl::currentChanged, this, [this](int index) {
        emit languageRequested(index == 1 ? QStringLiteral("en") : QStringLiteral("vi"));
    });
    connect(mLocation, &QAbstractButton::clicked, this, &LanguageLocationPage::locationRequested);
}

void LanguageLocationPage::setLanguage(const QString &language)
{
    mLanguage->setCurrentIndex(language == QLatin1String("en") ? 1 : 0);
}

void LanguageLocationPage::setLocationName(const QString &name)
{
    mLocation->setValue(name);
}
