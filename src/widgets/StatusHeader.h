#ifndef STATUS_HEADER_H
#define STATUS_HEADER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class StatusHeader; }
QT_END_NAMESPACE

class StatusHeader : public QWidget
{
    Q_OBJECT

public:
    explicit StatusHeader(QWidget *parent = nullptr);
    ~StatusHeader();

    void setOnlineStatus(bool online);
    void setVerifying(bool verifying);

private:
    Ui::StatusHeader *ui;
};

#endif // STATUS_HEADER_H
