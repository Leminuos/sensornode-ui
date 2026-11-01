#ifndef SEGMENTED_CONTROL_H
#define SEGMENTED_CONTROL_H

#include <QStringList>
#include <QWidget>

class QButtonGroup;

class SegmentedControl : public QWidget
{
    Q_OBJECT

public:
    explicit SegmentedControl(const QStringList &labels, QWidget *parent = nullptr);

    void setLabels(const QStringList &labels);
    int currentIndex() const;
    void setCurrentIndex(int index);

signals:
    void currentChanged(int index);

private:
    QButtonGroup *mGroup;
};

#endif // SEGMENTED_CONTROL_H
