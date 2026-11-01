#include "app/BootHoldDetector.h"

#include <QCoreApplication>
#include <QEvent>

BootHoldDetector::BootHoldDetector(QObject *parent, int windowMs, int holdMs)
    : QObject(parent)
{
    mWindow.setSingleShot(true);
    mWindow.setInterval(windowMs);
    connect(&mWindow, &QTimer::timeout, this, [this]() {
        if (!mHold.isActive()) {
            stop();
        }
    });

    mHold.setSingleShot(true);
    mHold.setInterval(holdMs);
    connect(&mHold, &QTimer::timeout, this, [this]() {
        stop();
        emit triggered();
    });
}

void BootHoldDetector::start()
{
    qApp->installEventFilter(this);
    mWindow.start();
}

void BootHoldDetector::stop()
{
    qApp->removeEventFilter(this);
    mWindow.stop();
    mHold.stop();
}

bool BootHoldDetector::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (mWindow.isActive() && !mHold.isActive()) {
            mHold.start();
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        const bool windowOver = !mWindow.isActive();
        mHold.stop();
        if (windowOver) {
            stop();
        }
    }
    return QObject::eventFilter(watched, event);
}
