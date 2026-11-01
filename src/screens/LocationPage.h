#ifndef LOCATION_PAGE_H
#define LOCATION_PAGE_H

#include <QVector>

#include "widgets/SubPage.h"

class ListRow;
class QLabel;
class ScrollList;

class LocationPage : public SubPage
{
    Q_OBJECT

public:
    explicit LocationPage(QWidget *parent = nullptr);

    int selectedIndex() const { return mSelected; }

public slots:
    void setSelectedIndex(int index);
    void finishApply(int previousIndex, bool ok);
    void setClockSynced(bool synced);
    void refreshTimes();

signals:
    void locationRequested(int index);

protected:
    void showEvent(QShowEvent *event) override;

private:
    ScrollList *mList;
    QLabel *mError;
    QVector<ListRow *> mRows;
    int mSelected = 0;
    bool mSynced = false;
};

#endif // LOCATION_PAGE_H
