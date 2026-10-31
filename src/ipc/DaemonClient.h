#ifndef DAEMON_CLIENT_H
#define DAEMON_CLIENT_H

#include <QByteArray>
#include <QJsonObject>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QTimer>

class DaemonClient : public QObject
{
    Q_OBJECT

public:
    explicit DaemonClient(const QString &socketPath, QObject *parent = nullptr);

    void start();
    bool isConnected() const;

    int sendCommand(const QString &cmd, QJsonObject args = QJsonObject());

signals:
    void connected();
    void disconnected();
    void messageReceived(const QJsonObject &message);

private:
    void onReadyRead();
    void onSocketLost(QLocalSocket *socket);
    void tryConnect();

    QString mPath;
    QLocalSocket *mSocket = nullptr;
    QTimer mReconnectTimer;
    QByteArray mBuffer;
    bool mOnline = false;
    int mNextId = 1;
};

#endif // DAEMON_CLIENT_H
