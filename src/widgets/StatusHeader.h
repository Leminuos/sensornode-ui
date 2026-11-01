#ifndef STATUS_HEADER_H
#define STATUS_HEADER_H

#include "model/NetworkModel.h"

#include <QWidget>

class QLabel;

class StatusHeader : public QWidget
{
    Q_OBJECT

public:
    explicit StatusHeader(QWidget *parent = nullptr);

public slots:
    // Chuỗi rỗng nghĩa là giờ chưa đồng bộ.
    void setClock(const QString &text);
    void setNetworkStatus(const NetworkStatus &status);
    void setVerifying(bool verifying);

private:
    QLabel *mClock;
    QLabel *mChip;
    QLabel *mEthernet;
    QLabel *mWifi;
    bool mVerifying = false;
};

#endif // STATUS_HEADER_H
