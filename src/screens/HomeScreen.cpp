#include "screens/HomeScreen.h"
#include "ui_HomeScreen.h"

#include <QtNumeric>

HomeScreen::HomeScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomeScreen)
{
    ui->setupUi(this);
}

HomeScreen::~HomeScreen()
{
    delete ui;
}

void HomeScreen::setValues(double tempC, double humidity, double lux)
{
    const QString none = QStringLiteral("--");
    ui->tempValue->setText(qIsNaN(tempC) ? none : QString::number(tempC, 'f', 1));
    ui->humValue->setText(qIsNaN(humidity) ? none : QString::number((int)humidity));
    ui->illValue->setText(qIsNaN(lux) ? none : QString::number((int)lux));
}
