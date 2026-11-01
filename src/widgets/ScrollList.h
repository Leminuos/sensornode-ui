#ifndef SCROLL_LIST_H
#define SCROLL_LIST_H

#include <QScrollArea>

class ScrollList : public QScrollArea
{
    Q_OBJECT

public:
    explicit ScrollList(QWidget *parent = nullptr);

    void setViewGeometry(const QRect &rect);
    void setContent(QWidget *content);
    void scrollToRow(int rowTop, int rowHeight);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void moveIndicator();

    QWidget *mIndicator;
};

#endif // SCROLL_LIST_H
