#include "screens/ServicePage.h"

#include "app/Language.h"
#include "widgets/ListCard.h"
#include "widgets/ListRow.h"

ServicePage::ServicePage(QWidget *parent)
    : SubPage(QString(), parent)
    , mCard(new ListCard(content()))
{
    ListRow *device = mCard->addRow(QStringLiteral("device"), QStringLiteral("blue"));
    ListRow *system = mCard->addRow(QStringLiteral("system"), QStringLiteral("aqua"));
    ListRow *network = mCard->addRow(QStringLiteral("network"), QStringLiteral("green"));
    mAdvanced = mCard->addRow(QStringLiteral("code"), QStringLiteral("default"));
    ListRow *factoryReset = mCard->addRow(QStringLiteral("restore"), QStringLiteral("red"));
    factoryReset->setDanger(true);
    mCard->place(8, 6);

    connect(device, &QAbstractButton::clicked, this, &ServicePage::deviceRequested);
    connect(system, &QAbstractButton::clicked, this, &ServicePage::systemRequested);
    connect(network, &QAbstractButton::clicked, this, &ServicePage::networkRequested);
    connect(mAdvanced, &QAbstractButton::clicked, this, &ServicePage::advancedRequested);
    connect(factoryReset, &QAbstractButton::clicked, this, &ServicePage::factoryResetRequested);

    Language::bind(this, [=]() {
        setTitle(tr("Bảo trì"));
        device->setTitle(tr("Thiết bị"));
        system->setTitle(tr("Hệ thống"));
        network->setTitle(tr("Mạng"));
        mAdvanced->setTitle(tr("Nâng cao"));
        factoryReset->setTitle(tr("Khôi phục cài đặt gốc"));
    });
}

void ServicePage::setAdvancedVisible(bool visible)
{
    mCard->setRowHidden(mAdvanced, !visible);
}
