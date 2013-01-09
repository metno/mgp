#include "qc.h"

#define MAXMSGSIZE 2048

QCChannel::QCChannel(QTcpSocket *socket)
    : socket(socket)
    , lastError_("<not set yet>")
{
    if (socket)
        initSocket();
}

QCChannel::~QCChannel()
{
    // socket->close(); ?
    delete socket;
}

QString QCChannel::lastError() const
{
    return lastError_;
}

void QCChannel::setLastError(const QString &lastError_)
{
    this->lastError_ = lastError_;
}

void QCChannel::initSocket()
{
    Q_ASSERT(socket);
    connect(
        socket, SIGNAL(error(QAbstractSocket::SocketError)),
        SLOT(handleSocketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), SIGNAL(socketDisconnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
    Q_ASSERT(!socket->peerAddress().isNull());
    Q_ASSERT(socket->peerPort() != 0);
    peerInfo_ = QString("%1:%2")
        .arg(QHostInfo::fromName(socket->peerAddress().toString()).hostName())
        .arg(socket->peerPort());
}

// Connects the channel to a server on \a host listening on \a port.
bool QCChannel::connectToServer(const QString &host, const quint16 port)
{
    if (socket) {
        setLastError("socket already connected, please disconnect first");
        return false;
    }

    socket = new QTcpSocket;
    socket->connectToHost(host, port);
    if (!socket->waitForConnected(-1)) {
        setLastError(QString("waitForConnected() failed: %1").arg(socket->errorString()));
        delete socket;
        socket = 0;
        return false;
    }

    initSocket();
    return true;
}

bool QCChannel::isConnected() const
{
    return socket != 0;
}

// Sends the string \a s in UTF-8 format on the channel.
void QCChannel::sendMessage(const QString &s)
{
    if (!socket) {
        const char *emsg = "socket not connected";
        qWarning("%s", emsg);
        setLastError(emsg);
        return;
    }

    QString msg = s.trimmed();

    if (msg.contains('\n')) {
        const char *emsg = "attempt to send message with internal newlines";
        qWarning("%s", emsg);
        setLastError(emsg);
        return;
    }

    if (msg.size() > MAXMSGSIZE) {
        qWarning("message too long; truncating to max %d characters", MAXMSGSIZE);
        msg.truncate(MAXMSGSIZE);
        msg = msg.trimmed();
    }

    msg.append('\n');

    socket->write(msg.toUtf8());
    socket->waitForBytesWritten(-1);
}

void QCChannel::readyRead()
{
    if (socket->bytesAvailable() == 0) {
        qDebug() << "warning: readyRead() called with no bytes available";
        return;
    }

    QByteArray msg = socket->readLine();
    emit messageArrived(QString::fromUtf8(msg).trimmed());
}

void QCChannel::handleSocketError(QAbstractSocket::SocketError)
{
    emit error(socket->errorString());
}

QCChannelServer::QCChannelServer()
    : lastError_("<not set yet>")
{
}

bool QCChannelServer::listen(const qint16 port)
{
    if (!server.listen(QHostAddress::Any, port)) {
        setLastError(QString("listen() failed: %1").arg(server.errorString()));
        return false;
    }
    connect(&server, SIGNAL(newConnection()), SLOT(newConnection()));
    qDebug() << "accepting client connections on port" << port << "...";
    return true;
}

QString QCChannelServer::lastError() const
{
    return lastError_;
}

void QCChannelServer::setLastError(const QString &lastError_)
{
    this->lastError_ = lastError_;
}

void QCChannelServer::newConnection()
{
    emit channelConnected(new QCChannel(server.nextPendingConnection()));
}

QCServerChannel::QCServerChannel()
    : channel(0)
    , lastError_("<not set yet>")
{
}

bool QCServerChannel::connectToServer(const QString &host, const quint16 port)
{
    if (channel) {
        lastError_ = "channel already connected, please disconnect first";
        return false;
    }

    channel = new QCChannel;

    if (!channel->connectToServer(host, port)) {
        lastError_ = channel->lastError();
        return false;
    }
    connect(channel, SIGNAL(messageArrived(const QString &)), SLOT(handleMessageArrived(const QString &)));
    connect(channel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
    connect(channel, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
    return true;
}

void QCServerChannel::showChatWindow()
{
    sendMessage("show_chat_win");
}

void QCServerChannel::hideChatWindow()
{
    sendMessage("hide_chat_win");
}

void QCServerChannel::sendNotification(const QString &msg)
{
    sendMessage(QString("notify %1").arg(msg));
}

QString QCServerChannel::lastError() const
{
    return lastError_;
}

void QCServerChannel::sendMessage(const QString &msg)
{
    if (!channel) {
        const char *emsg = "channel not connected";
        qWarning("%s", emsg);
        lastError_ = emsg;
        return;
    }
    channel->sendMessage(msg);
}

void QCServerChannel::handleMessageArrived(const QString &msg)
{
    if (msg == "chat_win_shown") {
        emit chatWindowShown(); // typically for updating button sensitivities
    } else if (msg == "chat_win_hidden") {
            emit chatWindowHidden(); // typically for updating button sensitivities
    } else {
        const QString nfKeyword("notify");
        if (msg.split(" ").first() == nfKeyword) {
            // typically for updating text in a status bar
            emit notification(msg.right(msg.size() - nfKeyword.size()).trimmed());
        }
    }
}

void QCServerChannel::handleChannelError(const QString &msg)
{
    qDebug() << "channel error:" << msg.toLatin1().data();
    lastError_ = msg;
}

void QCServerChannel::handleChannelDisconnected()
{
    Q_ASSERT(channel);
    qDebug() << "server disconnected:" << channel->peerInfo().toLatin1().data();
    channel->deleteLater();
    channel = 0;
    emit serverDisconnected();
}

QCClientChannels::QCClientChannels()
  : lastError_("<not set yet>")
{
}

bool QCClientChannels::listen(const qint16 port)
{
    if (!server.listen(port)) {
        lastError_ = QString("server.listen() failed: %1").arg(server.lastError());
        return false;
    }
    QObject::connect(&server, SIGNAL(channelConnected(QCChannel *)), SLOT(handleChannelConnected(QCChannel *)));
    return true;
}

void QCClientChannels::notifyChatWindowShown()
{
    broadcast("chat_win_shown");
}

void QCClientChannels::notifyChatWindowHidden()
{
    broadcast("chat_win_hidden");
}

void QCClientChannels::sendNotification(const QString &msg)
{
    broadcast(QString("notify %1").arg(msg));
}

QString QCClientChannels::lastError() const
{
    return lastError_;
}

void QCClientChannels::broadcast(const QString &msg)
{
    foreach (QCChannel *channel, channels)
        channel->sendMessage(msg);
}

void QCClientChannels::handleChannelConnected(QCChannel *channel)
{
    qDebug() << "new client connected:" << channel->peerInfo().toLatin1().data();
    connect(channel, SIGNAL(messageArrived(const QString &)), SLOT(handleMessageArrived(const QString &)));
    connect(channel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
    connect(channel, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
    channels.append(channel);
    emit clientConnected();
}

void QCClientChannels::handleMessageArrived(const QString &msg)
{
    if (msg == "show_chat_win") {
        emit showChatWindowRequested();
    } else if (msg == "hide_chat_win") {
        emit hideChatWindowRequested();
    } else {
        const QString nfKeyword("notify");
        if (msg.split(" ").first() == nfKeyword) {
            emit notificationRequested(msg.right(msg.size() - nfKeyword.size()).trimmed());
        }
    }
}

void QCClientChannels::handleChannelError(const QString &msg)
{
    qDebug() << "channel error:" << msg.toLatin1().data();
}

void QCClientChannels::handleChannelDisconnected()
{
    QCChannel *channel = static_cast<QCChannel *>(sender());
    qDebug() << "client disconnected:" << channel->peerInfo().toLatin1().data();
    channels.removeOne(channel);
    channel->deleteLater(); // ### or 'delete channel' directly?
}
