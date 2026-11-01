#include "screens/ServicePage.h"

#include "app/Language.h"
#include "app/Theme.h"
#include "widgets/ListRow.h"

#include <QFrame>
#include <QVariant>
#include <QVector>

static constexpr int kRowHeight = 44;

ServicePage::ServicePage(QWidget *parent)
    : SubPage(QString(), parent)
{
    QWidget *body = content();

    auto *card = new QFrame(body);
    card->setProperty("surface", "card");
    card->setGeometry(QRect(8, 6, 224, 5 * kRowHeight).marginsAdded(Theme::shadowMargins()));

    const struct { const char *glyph; const char *tile; void (ServicePage::*signal)(); } items[] = {
        {"cpu", "aqua", &ServicePage::firmwareRequested},
        {"network", "blue", &ServicePage::deviceRequested},
        {"sensor", "red", &ServicePage::sensorsRequested},
        {"crosshair", "yellow", &ServicePage::calibrationRequested},
        {"log", "default", &ServicePage::errorLogRequested},
    };

    int y = 6;
    QVector<ListRow *> rows;
    for (const auto &item : items) {
        auto *row = new ListRow(QString(), body);
        row->setLeadingIcon(QLatin1String(item.glyph), QLatin1String(item.tile));
        row->setGeometry(8, y, 224, kRowHeight);
        row->setSeparator(y != 6);
        connect(row, &QAbstractButton::clicked, this, item.signal);
        rows.append(row);
        y += kRowHeight;
    }

    Language::bind(this, [this, rows]() {
        setTitle(tr("Bảo trì"));
        rows.at(0)->setTitle(tr("Firmware & khởi động"));
        rows.at(1)->setTitle(tr("Thiết bị & mạng"));
        rows.at(2)->setTitle(tr("Cảm biến"));
        rows.at(3)->setTitle(tr("Hiệu chỉnh cảm ứng"));
        rows.at(4)->setTitle(tr("Nhật ký lỗi"));
    });
}
