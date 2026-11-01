#ifndef DISPLAY_MODEL_H
#define DISPLAY_MODEL_H

#include <QJsonObject>
#include <QObject>
#include <QString>

#include "ipc/DaemonClient.h"

class DisplayModel : public QObject
{
    Q_OBJECT

public:
    explicit DisplayModel(QObject *parent = nullptr, const QString &socketPath = QString());

    void start();

public slots:
    void setBrightness(int value);

private:
    void onMessage(const QJsonObject &message);

    DaemonClient mClient;
};

#endif // DISPLAY_MODEL_H
