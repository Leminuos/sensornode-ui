#ifndef FIRST_FRAME_NOTIFIER_H
#define FIRST_FRAME_NOTIFIER_H

#include <QObject>

class QWindow;

class FirstFrameNotifier : public QObject
{
    Q_OBJECT

public:
    explicit FirstFrameNotifier(QWindow *window);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif
