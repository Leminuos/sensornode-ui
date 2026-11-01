#include "screens/CalibrationIntroPage.h"

#include "app/Language.h"
#include "app/Theme.h"
#include "widgets/RaisedButton.h"

#include <QLabel>
#include <QPainter>

CalibrationIntroPage::CalibrationIntroPage(QWidget *parent)
    : SubPage(QString(), parent)
{
    QWidget *body = content();

    QPixmap haloImage(QStringLiteral(":/surfaces/halo_primary.png"));
    {
        QPainter painter(&haloImage);
        painter.drawPixmap(21, 19, Theme::icon(QStringLiteral("crosshair"), 34, QStringLiteral("on-primary")));
    }
    auto *halo = new QLabel(body);
    halo->setPixmap(haloImage);
    halo->setGeometry((240 - 76) / 2, 12, 76, 75);

    auto *title = new QLabel(body);
    title->setObjectName(QStringLiteral("flowTitle"));
    title->setAlignment(Qt::AlignCenter);
    title->setGeometry(0, 88, 240, 20);

    auto *note = new QLabel(body);
    note->setObjectName(QStringLiteral("flowBody"));
    note->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    note->setWordWrap(true);
    note->setGeometry(14, 116, 212, 48);

    auto *start = new RaisedButton(QString(), RaisedButton::Primary, body);
    start->setVisualGeometry(QRect(8, 208, 224, 42));
    connect(start, &QPushButton::clicked, this, &CalibrationIntroPage::startRequested);

    Language::bind(this, [=]() {
        setTitle(tr("Hiệu chỉnh cảm ứng"));
        title->setText(tr("Chạm vào 5 điểm"));
        note->setText(tr("Dùng đầu bút hoặc móng tay, chạm vào tâm dấu chữ thập."));
        start->setText(tr("Bắt đầu"));
    });
}
