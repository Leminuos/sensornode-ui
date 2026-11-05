#include "app/FirstFrameNotifier.h"

#include "ipc/SystemdNotify.h"

#include <QEvent>
#include <QTimer>
#include <QWindow>

FirstFrameNotifier::FirstFrameNotifier(QWindow *window)
    : QObject(window)
{
    window->installEventFilter(this);
}

bool FirstFrameNotifier::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Expose) {
        watched->removeEventFilter(this);
        QTimer::singleShot(0, this, []() { SystemdNotify::send("READY=1"); });
    }

    return QObject::eventFilter(watched, event);
}
