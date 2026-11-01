#ifndef SAMPLING_PAGE_H
#define SAMPLING_PAGE_H

#include <QVector>

#include "widgets/SubPage.h"

class ListRow;
class QLabel;

class SamplingPage : public SubPage
{
    Q_OBJECT

public:
    explicit SamplingPage(QWidget *parent = nullptr);

    int period() const { return mPeriodSec; }

    static QString periodText(int seconds);

public slots:
    void setPeriod(int seconds);
    void beginApply(int seconds);
    void finishApply(bool ok);

signals:
    void periodRequested(int seconds);

private:
    void refreshRows();

    QVector<ListRow *> mRows;
    QLabel *mError;
    int mPeriodSec = 5;
    int mPendingSec = 0;
};

#endif // SAMPLING_PAGE_H
