#ifndef SUB_PAGE_H
#define SUB_PAGE_H

#include <QWidget>

class QLabel;

// Khung trang con: thanh tiêu đề có nút quay lại, nội dung 240×256.
class SubPage : public QWidget
{
    Q_OBJECT

public:
    explicit SubPage(const QString &title, QWidget *parent = nullptr);

    void setTitle(const QString &title);
    QWidget *content() const { return mContent; }

signals:
    void backRequested();

private:
    QLabel *mTitle;
    QWidget *mContent;
};

#endif // SUB_PAGE_H
