#ifndef UPDATE_POPUP_H
#define UPDATE_POPUP_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class UpdatePopup; }
QT_END_NAMESPACE

class UpdatePopup : public QWidget
{
    Q_OBJECT

public:
    explicit UpdatePopup(QWidget *parent = nullptr);
    ~UpdatePopup();

    void setVersions(const QString &current, const QString &available);

    // Khoá nút Update now trong lúc bản vừa cài chưa được xác nhận.
    void setVerifying(bool verifying);

signals:
    void updateConfirmed();
    void cancelled();

private:
    Ui::UpdatePopup *ui;
};

#endif // UPDATE_POPUP_H
