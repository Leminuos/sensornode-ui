#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include <QWidget>

#include "model/Metric.h"

class ListRow;
class MiniSegment;

class SettingsScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsScreen(QWidget *parent = nullptr);

public slots:
    void setBrightness(int value);
    void setSamplingPeriodText(const QString &text);
    void setCurrentVersion(const QString &version);
    void setTemperatureUnit(TemperatureUnit unit);

signals:
    void displayRequested();
    void samplingRequested();
    void languageLocationRequested();
    void updateRequested();
    void serviceRequested();
    void temperatureUnitRequested(TemperatureUnit unit);

private:
    ListRow *mBrightness;
    ListRow *mSampling;
    ListRow *mUpdate;
    MiniSegment *mUnit;
};

#endif // SETTINGS_SCREEN_H
