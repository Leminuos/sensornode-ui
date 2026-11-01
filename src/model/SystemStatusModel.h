#ifndef SYSTEM_STATUS_MODEL_H
#define SYSTEM_STATUS_MODEL_H

#include <QObject>
#include <QString>
#include <QTimer>

class SystemStatusModel : public QObject
{
    Q_OBJECT

public:
    explicit SystemStatusModel(QObject *parent = nullptr, const QString &timeSyncFlag = QString(),
                               const QString &interfaceName = QString());

    void start();

    QString clockText() const { return mClockText; }
    bool networkUp() const { return mNetworkUp; }

signals:
    // Chuỗi rỗng nghĩa là giờ chưa đồng bộ.
    void clockChanged(const QString &text);
    void networkChanged(bool up);

private:
    void poll();

    QString mTimeSyncFlag;
    QString mInterfaceName;
    QString mClockText;
    bool mNetworkUp = false;
    QTimer mPoll;
};

#endif // SYSTEM_STATUS_MODEL_H
