#ifndef SENSORS_PAGE_H
#define SENSORS_PAGE_H

#include "model/Metric.h"
#include "model/Sample.h"
#include "widgets/SubPage.h"

class QLabel;
class QTimer;

class SensorsPage : public SubPage
{
    Q_OBJECT

public:
    explicit SensorsPage(QWidget *parent = nullptr);

public slots:
    void setReport(const SensorReport &report);
    void setPeriodText(const QString &text);
    void setTemperatureUnit(TemperatureUnit unit);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    struct Card
    {
        QLabel *chip;
        QLabel *note;
    };

    Card makeCard(int y, const QString &name, const QString &glyph, const QString &tile);
    void refresh();
    void applyCard(const Card &card, const QString &error, const QString &value);
    QString ageText() const;

    Card mSht30;
    Card mBh1750;
    QLabel *mPeriod;
    QTimer *mTick;
    SensorReport mReport;
    bool mHasReport = false;
    TemperatureUnit mUnit = TemperatureUnit::Celsius;
};

#endif // SENSORS_PAGE_H
