#ifndef KEY_VALUE_LIST_H
#define KEY_VALUE_LIST_H

#include <QFrame>
#include <QVector>

class QLabel;
class QProgressBar;

class KeyValueList : public QFrame
{
    Q_OBJECT

public:
    enum Tone { Normal, Good, Bad };

    explicit KeyValueList(QWidget *parent = nullptr);

    int addRow(const QString &label);
    int addChipRow(const QString &label);
    int addBarRow(const QString &label);

    void place(int x, int y);

    void setLabel(int row, const QString &text);
    void setValue(int row, const QString &text, Tone tone = Normal);
    // Cắt bằng dấu ba chấm để không đè lên nhãn.
    void setElidedValue(int row, const QString &text, Tone tone = Normal);
    void setChip(int row, const QString &text, const char *kind);
    void setBar(int row, int percent);

    QString value(int row) const;
    QString label(int row) const;

private:
    struct Row
    {
        QLabel *label;
        QLabel *value;
        QProgressBar *bar;
    };

    Row makeRow(const QString &label);

    QVector<Row> mRows;
    int mBottom;
};

#endif // KEY_VALUE_LIST_H
