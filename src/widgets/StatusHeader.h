#ifndef STATUS_HEADER_H
#define STATUS_HEADER_H

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
    void setNetworkUp(bool up);
    void setVerifying(bool verifying);

private:
    QLabel *mClock;
    QLabel *mChip;
    QLabel *mNetwork;
};

#endif // STATUS_HEADER_H
