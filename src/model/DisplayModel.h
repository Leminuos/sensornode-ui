#ifndef DISPLAY_MODEL_H
#define DISPLAY_MODEL_H

#include <QObject>
#include <QString>
#include <QTimer>

class DisplayModel : public QObject
{
    Q_OBJECT

public:
    explicit DisplayModel(QObject *parent = nullptr, const QString &devicePath = QString());

    // Áp mức mặc định nếu chưa có setBrightness nào sau 1 s.
    void start();

public slots:
    bool setBrightness(int value);

private:
    void report(bool ok);

    QString mBrightnessPath;
    QTimer mFallback;
    int mReported = -1;
};

#endif // DISPLAY_MODEL_H
