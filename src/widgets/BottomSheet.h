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
    void setAcceptEnabled(bool enabled);

    void setExtra(QWidget *widget, int height);
    QWidget *panel() const { return mPanel; }

    void present();
    void dismiss();

signals:
    void accepted();
    void rejected();

private:
    QWidget *mPanel;
    QLabel *mTitle;
    QLabel *mBody;
    QWidget *mExtra = nullptr;
    int mExtraHeight = 0;
    RaisedButton *mReject;
    RaisedButton *mAccept;
};

#endif // BOTTOM_SHEET_H
