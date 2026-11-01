#ifndef STEP_BAR_H
#define STEP_BAR_H

#include <QStringList>
#include <QVector>
#include <QWidget>

class StepBar : public QWidget
{
    Q_OBJECT

public:
    enum StepState { Pending, Active, Done, Failed };

    explicit StepBar(const QStringList &labels, QWidget *parent = nullptr);

    void setStates(const QVector<StepState> &states);
    StepState state(int step) const { return mStates.at(step); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QStringList mLabels;
    QVector<StepState> mStates;
};

#endif // STEP_BAR_H
