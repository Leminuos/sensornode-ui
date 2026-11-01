#ifndef ERROR_LOG_PAGE_H
#define ERROR_LOG_PAGE_H

#include <QFrame>
#include <QVector>

#include "model/ErrorLogModel.h"
#include "widgets/SubPage.h"

class QLabel;
class ScrollList;

class ErrorLogPage : public SubPage
{
    Q_OBJECT

public:
    explicit ErrorLogPage(QWidget *parent = nullptr);

    static QString timeLabel(const QDateTime &time, const QDateTime &now);

public slots:
    void setEntries(const QVector<LogEntry> &entries);
    void showLoadFailed();

signals:
    void refreshRequested();

protected:
    void showEvent(QShowEvent *event) override;

private:
    QFrame *mCard;
    ScrollList *mList;
    QLabel *mMessage;
};

#endif // ERROR_LOG_PAGE_H
