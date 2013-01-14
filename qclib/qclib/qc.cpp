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

// Sends the variant map \a vmap as a message on the channel.
void QCChannel::sendMessage(const QVariantMap &vmap)
{
    if (!socket) {
        const char *emsg = "socket not connected";
        qWarning("%s", emsg);
        setLastError(emsg);
        return;
    }

    // serialize into byte array
    QByteArray ba;
    QDataStream dstream(&ba, QIODevice::Append);
    dstream << vmap;

    // send as newline-terminated base64
    socket->write(ba.toBase64());
    socket->write("\n");
    socket->waitForBytesWritten(-1);
}

void QCChannel::readyRead()
{
    if (socket->bytesAvailable() == 0) {
        qDebug() << "warning: readyRead() called with no bytes available";
        return;
    }

    // receive as newline-terminated base64
    msgbuf_.append(socket->readLine());
    if (msgbuf_.at(msgbuf_.size() - 1) != '\n')
        return; // message not complete yet

    msgbuf_.chop(1); // strip trailing newline
    QByteArray ba = QByteArray::fromBase64(msgbuf_); // decode
    msgbuf_.clear(); // prepare reception of next message

    // deserialize into variant map
    QDataStream dstream(ba);
    QVariantMap msg;
    dstream >> msg;

    emit messageArrived(msg);
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

QCBase::QCBase()
    : lastError_("<not set yet>")
{
}

void QCBase::setLastError(const QString &error)
{
    lastError_ = error;
}

QString QCBase::lastError() const
{
    return lastError_;
}

void QCBase::showChatWindow()
{
    QVariantMap msg;
    msg.insert("type", ShowChatWin);
    // add more fields as necessary ... 2 B DONE
    sendMessage(msg);
}

void QCBase::hideChatWindow()
{
    QVariantMap msg;
    msg.insert("type", HideChatWin);
    // add more fields as necessary ... 2 B DONE
    sendMessage(msg);
}

void QCBase::sendChatMessage(const QString &m)
{
    QVariantMap msg;
    msg.insert("type", ChatMsg);
    msg.insert("value", m);
    // add more fields as necessary ... 2 B DONE
    sendMessage(msg);
}

void QCBase::sendNotification(const QString &m)
{
    QVariantMap msg;
    msg.insert("type", Notification);
    msg.insert("value", m);
    // add more fields as necessary ... 2 B DONE
    sendMessage(msg);
}

void QCBase::handleMessageArrived(const QVariantMap &msg)
{
    Q_ASSERT(!msg.contains("type"));
    bool ok;
    const MsgType type = static_cast<MsgType>(msg.value("type").toInt(&ok));
    if (!ok) {
        qDebug() << "vmap:" << msg;
        qFatal("failed to convert message type to int: %s", msg.value("type").toString().toLatin1().data());
    }
    if (type == ShowChatWin) {
        // pass more fields as necessary ... 2 B DONE
        emit chatWindowShown();
    } else if (type == HideChatWin) {
        // pass more fields as necessary ... 2 B DONE
        emit chatWindowHidden();
    } else if (type == ChatMsg) {
        Q_ASSERT(msg.value("value").canConvert(QVariant::String));
        // pass more fields as necessary ... 2 B DONE
        emit chatMessage(msg.value("value").toString());
    } else if (type == Notification) {
        Q_ASSERT(msg.value("value").canConvert(QVariant::String));
        // pass more fields as necessary ... 2 B DONE
        emit notification(msg.value("value").toString());
    } else {
        qFatal("invalid message type: %d", type);
    }
}

void QCBase::handleChannelError(const QString &msg)
{
    qDebug() << "channel error:" << msg.toLatin1().data();
    lastError_ = msg;
}


QCServerChannel::QCServerChannel()
    : channel(0)
{
}

bool QCServerChannel::connectToServer(const QString &host, const quint16 port)
{
    if (channel) {
        setLastError("channel already connected, please disconnect first");
        return false;
    }

    channel = new QCChannel;

    if (!channel->connectToServer(host, port)) {
        setLastError(channel->lastError());
        return false;
    }
    connect(channel, SIGNAL(messageArrived(const QVariantMap &)), SLOT(handleMessageArrived(const QVariantMap &)));
    connect(channel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
    connect(channel, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
    return true;
}

void QCServerChannel::sendMessage(const QVariantMap &msg)
{
    if (!channel) {
        const char *emsg = "channel not connected";
        qWarning("%s", emsg);
        setLastError(emsg);
        return;
    }
    channel->sendMessage(msg);
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
{
}

bool QCClientChannels::listen(const qint16 port)
{
    if (!server.listen(port)) {
        setLastError(QString("server.listen() failed: %1").arg(server.lastError()));
        return false;
    }
    QObject::connect(&server, SIGNAL(channelConnected(QCChannel *)), SLOT(handleChannelConnected(QCChannel *)));
    return true;
}

void QCClientChannels::sendMessage(const QVariantMap &msg)
{
    foreach (QCChannel *channel, channels)
        channel->sendMessage(msg);
}

void QCClientChannels::handleChannelConnected(QCChannel *channel)
{
    qDebug() << "new client connected:" << channel->peerInfo().toLatin1().data();
    connect(channel, SIGNAL(messageArrived(const QVariantMap &)), SLOT(handleMessageArrived(const QVariantMap &)));
    connect(channel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
    connect(channel, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
    channels.append(channel);
    emit clientConnected();
}

void QCClientChannels::handleChannelDisconnected()
{
    QCChannel *channel = static_cast<QCChannel *>(sender());
    qDebug() << "client disconnected:" << channel->peerInfo().toLatin1().data();
    channels.removeOne(channel);
    channel->deleteLater(); // ### or 'delete channel' directly?
}
