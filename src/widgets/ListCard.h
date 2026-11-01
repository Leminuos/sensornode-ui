#ifndef LIST_CARD_H
#define LIST_CARD_H

#include <QFrame>
#include <QPoint>
#include <QVector>

class ListRow;

// Thẻ danh sách dòng 44 px, dòng bị ẩn không chiếm chỗ.
class ListCard : public QFrame
{
    Q_OBJECT

public:
    explicit ListCard(QWidget *parent = nullptr);

    ListRow *addRow(const QString &glyph, const QString &tile);
    void setRowHidden(ListRow *row, bool hidden);
    void place(int x, int y);

private:
    void layoutRows();

    QVector<ListRow *> mRows;
    QPoint mOrigin;
};

#endif // LIST_CARD_H
