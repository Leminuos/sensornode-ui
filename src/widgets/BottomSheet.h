#ifndef BOTTOM_SHEET_H
#define BOTTOM_SHEET_H

#include <QWidget>

#include "widgets/RaisedButton.h"

class QLabel;

class BottomSheet : public QWidget
{
    Q_OBJECT

public:
    explicit BottomSheet(QWidget *parent);

    void setTitle(const QString &title);
    void setBody(const QString &body);
    void setActions(const QString &rejectText, const QString &acceptText, RaisedButton::Variant acceptVariant);

    void present();

signals:
    void accepted();
    void rejected();

private:
    QWidget *mPanel;
    QLabel *mTitle;
    QLabel *mBody;
    RaisedButton *mReject;
    RaisedButton *mAccept;
};

#endif // BOTTOM_SHEET_H
