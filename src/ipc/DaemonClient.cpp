#include "ipc/DaemonClient.h"

#include <QDebug>
#include <QJsonDocument>

static const int kReconnectIntervalMs = 2000;
static const int kMaxLineBytes = 64 * 1024;

DaemonClient::DaemonClient(const QString &socketPath, QObject *parent)
    : QObject(parent)
    , mPath(socketPath)
{
    mReconnectTimer.setSingleShot(true);
    mReconnectTimer.setInterval(kReconnectIntervalMs);
    connect(&mReconnectTimer, &QTimer::timeout, this, &DaemonClient::tryConnect);
}

void DaemonClient::start()
{
    tryConnect();
}

bool DaemonClient::isConnected() const
{
    return mOnline;
}

int DaemonClient::sendCommand(const QString &cmd, QJsonObject args)
{
    if (!mOnline) return -1;

    const int id = mNextId++;
    args.insert(QStringLiteral("cmd"), cmd);
    args.insert(QStringLiteral("id"), id);
    mSocket->write(QJsonDocument(args).toJson(QJsonDocument::Compact) + '\n');
    return id;
}

void DaemonClient::tryConnect()
{
    mBuffer.clear();

    QLocalSocket *socket = new QLocalSocket(this);
    mSocket = socket;
    connect(socket, &QLocalSocket::connected, this, [this]() {
        mOnline = true;
        emit connected();
    });
    connect(socket, &QLocalSocket::readyRead, this, &DaemonClient::onReadyRead);
    connect(socket, &QLocalSocket::disconnected, this, [this, socket]() { onSocketLost(socket); });
    connect(socket, &QLocalSocket::errorOccurred, this, [this, socket]() { onSocketLost(socket); });
    socket->connectToServer(mPath);
}

void DaemonClient::onSocketLost(QLocalSocket *socket)
{
    if (socket != mSocket) return;
    
    socket->disconnect(this);
    socket->deleteLater();
    mSocket = nullptr;

    if (mOnline) {
        mOnline = false;
        emit disconnected();
    }

    if (!mReconnectTimer.isActive()) {
        mReconnectTimer.start();
    }
}

void DaemonClient::onReadyRead()
{
    mBuffer.append(mSocket->readAll());

    int nl;
    while ((nl = mBuffer.indexOf('\n')) >= 0) {
        const QByteArray line = mBuffer.left(nl);
        mBuffer.remove(0, nl + 1);

        const QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) {
            qWarning() << "DaemonClient:" << mPath << "invalid line" << line.left(80);
            continue;
        }
        
        emit messageReceived(doc.object());
    }

    if (mBuffer.size() > kMaxLineBytes) {
        qWarning() << "DaemonClient:" << mPath << "line too long, dropping";
        mBuffer.clear();
    }
}
