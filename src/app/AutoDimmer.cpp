#include "app/AutoDimmer.h"

#include <QCoreApplication>
#include <QEvent>

static bool isPress(QEvent::Type type)
{
    return type == QEvent::MouseButtonPress || type == QEvent::TouchBegin || type == QEvent::KeyPress;
}

static bool isRelease(QEvent::Type type)
{
    return type == QEvent::MouseButtonRelease || type == QEvent::TouchEnd || type == QEvent::TouchCancel || type == QEvent::KeyRelease;
}

static bool isInput(QEvent::Type type)
{
    return isPress(type) || isRelease(type) || type == QEvent::MouseMove || type == QEvent::TouchUpdate || type == QEvent::MouseButtonDblClick;
}

AutoDimmer::AutoDimmer(QObject *parent) : QObject(parent)
{
    mTimer.setSingleShot(true);
    connect(&mTimer, &QTimer::timeout, this, [this]() { setOff(true); });
    if (QCoreApplication *app = QCoreApplication::instance()) {
        app->installEventFilter(this);
    }
}

void AutoDimmer::setTimeoutMinutes(int minutes)
{
    setTimeoutMs(qMax(0, minutes) * 60 * 1000);
}

void AutoDimmer::setTimeoutMs(int ms)
{
    mTimeoutMs = qMax(0, ms);
    wake();
}

void AutoDimmer::setInhibited(bool inhibited)
{
    mInhibited = inhibited;
    wake();
}

void AutoDimmer::wake()
{
    setOff(false);
    restart();
}

void AutoDimmer::restart()
{
    if (mTimeoutMs > 0 && !mInhibited) mTimer.start(mTimeoutMs);
    else mTimer.stop();
}

void AutoDimmer::setOff(bool off)
{
    if (off == mOff) return;

    mOff = off;
    emit offChanged(off);
}

bool AutoDimmer::eventFilter(QObject *watched, QEvent *event)
{
    const QEvent::Type type = event->type();
    if (!isInput(type)) return QObject::eventFilter(watched, event);

    if (mOff && isPress(type)) {
        mSwallowing = true;
        wake();
    }

    if (mSwallowing) {
        if (isRelease(type)) mSwallowing = false;
        return true;
    }

    restart();
    return QObject::eventFilter(watched, event);
}
