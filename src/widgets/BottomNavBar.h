#ifndef BOTTOM_NAV_BAR_H
#define BOTTOM_NAV_BAR_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class BottomNavBar; }
QT_END_NAMESPACE

class QButtonGroup;

// Thanh điều hướng: Home / Dashboard / Settings
class BottomNavBar : public QWidget
{
    Q_OBJECT

public:
    enum Tab { Home = 0, Dashboard = 1, Settings = 2 };

    explicit BottomNavBar(QWidget *parent = nullptr);
    ~BottomNavBar();

public slots:
    void setActive(int index);

signals:
    void navRequested(int index);

private:
    Ui::BottomNavBar *ui;
    QButtonGroup *mGroup;
};

#endif // BOTTOM_NAV_BAR_H
