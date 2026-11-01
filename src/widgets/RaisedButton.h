#ifndef RAISED_BUTTON_H
#define RAISED_BUTTON_H

#include <QPushButton>

class RaisedButton : public QPushButton
{
    Q_OBJECT

public:
    enum Variant { Primary, Secondary, Danger };

    RaisedButton(const QString &text, Variant variant, QWidget *parent = nullptr);

    // Nhận hình chữ nhật của mặt nút, không gồm vùng bóng. Dùng thay cho setGeometry().
    void setVisualGeometry(const QRect &rect);

protected:
    bool hitButton(const QPoint &pos) const override;
};

#endif // RAISED_BUTTON_H
