#ifndef BOTTOM_NAV_BAR_H
#define BOTTOM_NAV_BAR_H

#include <QWidget>

class QButtonGroup;

// Thanh điều hướng: Home / Dashboard / Settings
class BottomNavBar : public QWidget
{
    Q_OBJECT

public:
    enum Tab { Home = 0, Dashboard = 1, Settings = 2 };

    explicit BottomNavBar(QWidget *parent = nullptr);

public slots:
    void setActive(int index);

signals:
    void navRequested(int index);

private:
    QButtonGroup *mGroup;
};

#endif // BOTTOM_NAV_BAR_H
