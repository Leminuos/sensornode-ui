#ifndef SYSTEM_STATUS_MODEL_H
#define SYSTEM_STATUS_MODEL_H

#include <QObject>
#include <QString>
#include <QTimer>

class SystemStatusModel : public QObject
{
    Q_OBJECT

public:
    explicit SystemStatusModel(QObject *parent = nullptr, const QString &timeSyncFlag = QString());

    void start();

    QString clockText() const { return mClockText; }

public slots:
    void refresh();

signals:
    void clockChanged(const QString &text);

private:
    QString mTimeSyncFlag;
    QString mClockText;
    QTimer mPoll;
};

#endif // SYSTEM_STATUS_MODEL_H
