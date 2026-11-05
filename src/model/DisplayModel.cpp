#include "model/DisplayModel.h"

#include "ipc/DaemonEndpoints.h"
#include "ipc/SystemdNotify.h"

#include <QDebug>
#include <QFile>

static constexpr int kDefaultBrightness = 72;
static constexpr int kFallbackMs = 1000;

// Slider 0..100 -> duty %, theo CIE: Y = ((L+16)/116)^3 nếu L>8, ngược lại Y = L/903.3.
static constexpr int kBrightnessLut[101] = {
     10,  10,  10,  10,  10,  10,  11,  11,  11,  11,  // 0..9
     11,  11,  11,  11,  12,  12,  12,  12,  12,  12,  // 10..19
     13,  13,  13,  13,  14,  14,  14,  15,  15,  15,  // 20..29
     16,  16,  16,  17,  17,  18,  18,  19,  19,  20,  // 30..39
     20,  21,  21,  22,  22,  23,  24,  24,  25,  26,  // 40..49
     27,  27,  28,  29,  30,  31,  32,  32,  33,  34,  // 50..59
     35,  36,  37,  38,  40,  41,  42,  43,  44,  45,  // 60..69
     47,  48,  49,  51,  52,  53,  55,  56,  58,  59,  // 70..79
     61,  63,  64,  66,  68,  69,  71,  73,  75,  77,  // 80..89
     79,  81,  83,  85,  87,  89,  91,  93,  95,  98,  // 90..99
    100,                                               // 100
};

DisplayModel::DisplayModel(QObject *parent, const QString &devicePath)
    : QObject(parent)
    , mBrightnessPath((devicePath.isEmpty() ? DaemonEndpoints::backlightDevice() : devicePath)
                      + QStringLiteral("/brightness"))
    , mBrightness(kDefaultBrightness)
{
    mFallback.setSingleShot(true);
    mFallback.setInterval(kFallbackMs);
    connect(&mFallback, &QTimer::timeout, this, [this]() { setBrightness(kDefaultBrightness); });
}

void DisplayModel::start()
{
    if (mReported < 0) mFallback.start();
}

bool DisplayModel::setBrightness(int value)
{
    mFallback.stop();
    mBrightness = qBound(0, value, 100);
    return mOff || write(kBrightnessLut[mBrightness]);
}

bool DisplayModel::setOff(bool off)
{
    if (off == mOff) return true;

    mOff = off;
    return write(off ? 0 : kBrightnessLut[mBrightness]);
}

bool DisplayModel::write(int duty)
{
    QFile file(mBrightnessPath);
    const QByteArray text = QByteArray::number(duty);
    const bool ok = file.open(QFile::WriteOnly | QFile::Truncate) && file.write(text) == text.size() && file.flush();
    if (!ok) {
        qWarning() << "Backlight: write failed" << mBrightnessPath << file.errorString();
    }
    report(ok);
    return ok;
}

// ota-confirm-boot đọc backlight= trong STATUS để quyết định commit slot.
void DisplayModel::report(bool ok)
{
    if (mReported == int(ok)) return;

    mReported = int(ok);
    SystemdNotify::send(ok ? "STATUS=backlight=ok" : "STATUS=backlight=error");
}
