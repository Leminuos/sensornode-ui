#include "screens/ServicePage.h"

#include "app/Theme.h"
#include "widgets/ListRow.h"

#include <QFrame>
#include <QVariant>

static constexpr int kRowHeight = 44;

ServicePage::ServicePage(QWidget *parent)
    : SubPage(tr("Bảo trì"), parent)
{
    QWidget *body = content();

    auto *card = new QFrame(body);
    card->setProperty("surface", "card");
    card->setGeometry(QRect(8, 6, 224, 5 * kRowHeight).marginsAdded(Theme::shadowMargins()));

    const struct { QString title; const char *glyph; const char *tile; void (ServicePage::*signal)(); } rows[] = {
        {tr("Firmware & khởi động"), "cpu", "aqua", &ServicePage::firmwareRequested},
        {tr("Thiết bị & mạng"), "network", "blue", &ServicePage::deviceRequested},
        {tr("Cảm biến"), "sensor", "red", &ServicePage::sensorsRequested},
        {tr("Hiệu chỉnh cảm ứng"), "crosshair", "yellow", &ServicePage::calibrationRequested},
        {tr("Nhật ký lỗi"), "log", "default", &ServicePage::errorLogRequested},
    };

    int y = 6;
    for (const auto &item : rows) {
        auto *row = new ListRow(item.title, body);
        row->setLeadingIcon(QLatin1String(item.glyph), QLatin1String(item.tile));
        row->setGeometry(8, y, 224, kRowHeight);
        row->setSeparator(y != 6);
        connect(row, &QAbstractButton::clicked, this, item.signal);
        y += kRowHeight;
    }
}
