#include "screens/SettingsScreen.h"

#include "app/Theme.h"
#include "widgets/ListRow.h"
#include "widgets/MiniSegment.h"

#include <QFrame>
#include <QVariant>

static constexpr int kRowHeight = 40;
static constexpr int kCardY = 3;

SettingsScreen::SettingsScreen(QWidget *parent)
    : QWidget(parent)
    , mBrightness(new ListRow(tr("Độ sáng"), this))
    , mSampling(new ListRow(tr("Chu kỳ lấy mẫu"), this))
    , mUpdate(new ListRow(tr("Cập nhật"), this))
{
    auto *card = new QFrame(this);
    card->setProperty("surface", "card");
    card->setGeometry(QRect(8, kCardY, 224, 6 * kRowHeight).marginsAdded(Theme::shadowMargins()));
    card->lower();

    auto *unitRow = new ListRow(tr("Đơn vị nhiệt độ"), this);
    auto *languageRow = new ListRow(tr("Ngôn ngữ & vị trí"), this);
    auto *serviceRow = new ListRow(tr("Bảo trì"), this);

    mBrightness->setLeadingIcon(QStringLiteral("sun"), QStringLiteral("yellow"));
    mSampling->setLeadingIcon(QStringLiteral("clock"), QStringLiteral("aqua"));
    unitRow->setLeadingIcon(QStringLiteral("thermometer"), QStringLiteral("red"));
    languageRow->setLeadingIcon(QStringLiteral("globe"), QStringLiteral("blue"));
    mUpdate->setLeadingIcon(QStringLiteral("download"), QStringLiteral("green"));
    serviceRow->setLeadingIcon(QStringLiteral("wrench"), QStringLiteral("default"));

    unitRow->setTrailing(ListRow::NoTrailing);
    unitRow->setPressable(false);

    int y = kCardY;
    for (ListRow *row : {mBrightness, mSampling, unitRow, languageRow, mUpdate, serviceRow}) {
        row->setGeometry(8, y, 224, kRowHeight);
        y += kRowHeight;
    }
    mBrightness->setSeparator(false);

    mUnit = new MiniSegment(QStringLiteral("°C"), QStringLiteral("°F"), unitRow);
    mUnit->move(unitRow->width() - mUnit->width(), 0);

    connect(mBrightness, &QAbstractButton::clicked, this, &SettingsScreen::displayRequested);
    connect(mSampling, &QAbstractButton::clicked, this, &SettingsScreen::samplingRequested);
    connect(languageRow, &QAbstractButton::clicked, this, &SettingsScreen::languageLocationRequested);
    connect(mUpdate, &QAbstractButton::clicked, this, &SettingsScreen::updateRequested);
    connect(serviceRow, &QAbstractButton::clicked, this, &SettingsScreen::serviceRequested);
    connect(mUnit, &MiniSegment::currentChanged, this, [this](int index) {
        emit temperatureUnitRequested(index == 1 ? TemperatureUnit::Fahrenheit : TemperatureUnit::Celsius);
    });
}

void SettingsScreen::setBrightness(int value)
{
    mBrightness->setValue(QStringLiteral("%1%").arg(value));
}

void SettingsScreen::setSamplingPeriodText(const QString &text)
{
    mSampling->setValue(text);
}

void SettingsScreen::setCurrentVersion(const QString &version)
{
    mUpdate->setValue(version);
}

void SettingsScreen::setTemperatureUnit(TemperatureUnit unit)
{
    mUnit->setCurrentIndex(unit == TemperatureUnit::Fahrenheit ? 1 : 0);
}
