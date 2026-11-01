#ifndef MINI_SEGMENT_H
#define MINI_SEGMENT_H

#include <QWidget>

class QButtonGroup;

class MiniSegment : public QWidget
{
    Q_OBJECT

public:
    MiniSegment(const QString &left, const QString &right, QWidget *parent = nullptr);

    int currentIndex() const;
    void setCurrentIndex(int index);

signals:
    void currentChanged(int index);

private:
    QButtonGroup *mGroup;
};

#endif // MINI_SEGMENT_H
