#include "screens/UpdatePopup.h"
#include "ui_UpdatePopup.h"

#include <QPushButton>

UpdatePopup::UpdatePopup(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UpdatePopup)
{
    ui->setupUi(this);

    QObject::connect(ui->updateNowButton, &QPushButton::clicked, [this]() {
        emit updateConfirmed();
        hide();
    });
    
    QObject::connect(ui->cancelUpdateButton, &QPushButton::clicked, [this]() {
        emit cancelled();
        hide();
    });
}

UpdatePopup::~UpdatePopup()
{
    delete ui;
}

void UpdatePopup::setVersions(const QString &current, const QString &available)
{
    ui->firmwareOldVersion->setText(current);
    ui->firmwareNewVersion->setText(available);
}

void UpdatePopup::setVerifying(bool verifying)
{
    ui->updateNowButton->setEnabled(!verifying);
    ui->updateNowButton->setText(verifying ? "Verifying…" : "Update now");
}
