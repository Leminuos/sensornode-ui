#include "screens/AdvancedPage.h"

#include "app/Language.h"
#include "widgets/ListCard.h"
#include "widgets/ListRow.h"

AdvancedPage::AdvancedPage(QWidget *parent)
    : SubPage(QString(), parent)
{
    auto *card = new ListCard(content());
    ListRow *firmware = card->addRow(QStringLiteral("cpu"), QStringLiteral("aqua"));
    ListRow *sensors = card->addRow(QStringLiteral("sensor"), QStringLiteral("red"));
    ListRow *calibration = card->addRow(QStringLiteral("crosshair"), QStringLiteral("yellow"));
    ListRow *errorLog = card->addRow(QStringLiteral("log"), QStringLiteral("default"));
    card->place(8, 6);

    connect(firmware, &QAbstractButton::clicked, this, &AdvancedPage::firmwareRequested);
    connect(sensors, &QAbstractButton::clicked, this, &AdvancedPage::sensorsRequested);
    connect(calibration, &QAbstractButton::clicked, this, &AdvancedPage::calibrationRequested);
    connect(errorLog, &QAbstractButton::clicked, this, &AdvancedPage::errorLogRequested);

    Language::bind(this, [=]() {
        setTitle(tr("Nâng cao"));
        firmware->setTitle(tr("Firmware & khởi động"));
        sensors->setTitle(tr("Cảm biến"));
        calibration->setTitle(tr("Hiệu chỉnh cảm ứng"));
        errorLog->setTitle(tr("Nhật ký lỗi"));
    });
}
