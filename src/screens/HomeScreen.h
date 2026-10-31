#ifndef HOME_SCREEN_H
#define HOME_SCREEN_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class HomeScreen; }
QT_END_NAMESPACE

class HomeScreen : public QWidget
{
    Q_OBJECT

public:
    HomeScreen(QWidget *parent = nullptr);
    ~HomeScreen();

public slots:
    void setValues(double tempC, double humidity, double lux);

private:
    Ui::HomeScreen *ui;
};
#endif // HOME_SCREEN_H
