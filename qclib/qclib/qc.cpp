#include "qc.h"

QCChannel::QCChannel(QTcpSocket *socket)
    : id_(nextId_++)
    , socket_(socket)
    , lastError_("<not set yet>")
{
    if (socket_)
        initSocket();
}

QCChannel::~QCChannel()
{
    // socket_->close(); ?
    delete socket_;
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
        socket_, SIGNAL(error(QAbstractSocket::SocketError)),
        SLOT(handleSocketError(QAbstractSocket::SocketError)));
    connect(socket_, SIGNAL(disconnected()), SIGNAL(socketDisconnected()));
    connect(socket_, SIGNAL(readyRead()), SLOT(readyRead()));
    Q_ASSERT(!socket_->peerAddress().isNull());
    Q_ASSERT(socket_->peerPort() != 0);
    peerInfo_ = QString("%1:%2")
        .arg(QHostInfo::fromName(socket_->peerAddress().toString()).hostName())
        .arg(socket_->peerPort());
}

// Connects the channel to a server on \a host listening on \a port.
bool QCChannel::connectToServer(const QString &host, const quint16 port)
{
    if (socket_) {
        setLastError("socket already connected, please disconnect first");
        return false;
    }

    socket_ = new QTcpSocket;
    socket_->connectToHost(host, port);
    if (!socket_->waitForConnected(-1)) {
        setLastError(QString("waitForConnected() failed: %1").arg(socket_->errorString()));
        delete socket_;
        socket_ = 0;
        return false;
    }

    initSocket();
    return true;
}

bool QCChannel::isConnected() const
{
    return socket_ != 0;
}

// Sends the variant map \a msg as a message on the channel.
void QCChannel::sendMessage(const QVariantMap &msg)
{
    if (!socket_) {
        const char *emsg = "socket not connected";
        qWarning("WARNING: %s", emsg);
        setLastError(emsg);
        return;
    }

    // serialize into byte array
    QByteArray ba;
    QDataStream dstream(&ba, QIODevice::Append);
    dstream << msg;

    // send as newline-terminated base64
    socket_->write(ba.toBase64());
    socket_->write("\n");
    socket_->waitForBytesWritten(-1);
}

void QCChannel::readyRead()
{
    if (socket_->bytesAvailable() == 0) {
        qDebug() << "warning: readyRead() called with no bytes available";
        return;
    }

    // receive as newline-terminated base64
    msgbuf_.append(socket_->readLine());
    if (msgbuf_.at(msgbuf_.size() - 1) != '\n')
        return; // message not complete yet

    msgbuf_.chop(1); // strip trailing newline
    QByteArray ba = QByteArray::fromBase64(msgbuf_); // decode
    msgbuf_.clear(); // prepare reception of next message

    // deserialize into variant map
    QDataStream dstream(ba);
    QVariantMap msg;
    dstream >> msg;

    //qDebug() << "msg arrived:" << msg;
    emit messageArrived(id(), msg);
}

void QCChannel::handleSocketError(QAbstractSocket::SocketError)
{
    emit error(socket_->errorString());
}

qint64 QCChannel::nextId_ = 0;

QCChannelServer::QCChannelServer()
    : lastError_("<not set yet>")
{
}

bool QCChannelServer::listen(const qint16 port)
{
    if (!server_.listen(QHostAddress::Any, port)) {
        setLastError(QString("listen() failed: %1").arg(server_.errorString()));
        return false;
    }
    connect(&server_, SIGNAL(newConnection()), SLOT(newConnection()));
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
    emit channelConnected(new QCChannel(server_.nextPendingConnection()));
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

void QCBase::showChatWindow(qint64 qcapp)
{
    QVariantMap msg;
    msg.insert("type", ShowChatWin);
    msg.insert("client", qcapp);
    sendMessage(msg);
}

void QCBase::hideChatWindow(qint64 qcapp)
{
    QVariantMap msg;
    msg.insert("type", HideChatWin);
    msg.insert("client", qcapp);
    sendMessage(msg);
}

void QCBase::sendChatMessage(const QString &text, const QString &user, int timestamp)
{
    QVariantMap msg;
    msg.insert("text", text);
    msg.insert("user", user);
    msg.insert("timestamp", timestamp);
    msg.insert("type", ChatMsg);
    sendMessage(msg);
}

void QCBase::sendNotification(const QString &text, const QString &user, int timestamp)
{
    QVariantMap msg;
    msg.insert("text", text);
    msg.insert("user", user);
    msg.insert("timestamp", timestamp);
    msg.insert("type", Notification);
    sendMessage(msg);
}

void QCBase::handleMessageArrived(qint64 channelId, const QVariantMap &msg)
{
    Q_ASSERT(!msg.contains("type"));
    bool ok;
    const MsgType type = static_cast<MsgType>(msg.value("type").toInt(&ok));
    if (!ok) {
        qDebug() << "msg:" << msg;
        qFatal("failed to convert message type to int: %s", msg.value("type").toString().toLatin1().data());
    }
    if (type == ShowChatWin) {
        emit chatWindowShown();
    } else if (type == HideChatWin) {
        emit chatWindowHidden();
    } else if (type == ChatMsg) {
        Q_ASSERT(msg.value("text").canConvert(QVariant::String));
        Q_ASSERT(msg.value("user").canConvert(QVariant::String));
        Q_ASSERT(msg.value("timestamp").canConvert(QVariant::Int));
        emit chatMessage(
            msg.value("text").toString(), msg.value("user").toString(), msg.value("timestamp").toInt());
    } else if (type == Notification) {
        Q_ASSERT(msg.value("text").canConvert(QVariant::String));
        Q_ASSERT(msg.value("user").canConvert(QVariant::String));
        Q_ASSERT(msg.value("timestamp").canConvert(QVariant::Int));
        emit notification(
            msg.value("text").toString(), msg.value("user").toString(), msg.value("timestamp").toInt());
    } else if (type == HistoryRequest) {
        emit historyRequest(channelId);
    } else if (type == History) {
        Q_ASSERT(msg.value("value").canConvert(QVariant::StringList));
        emit history(msg.value("value").toStringList());
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
    connect(
        channel, SIGNAL(messageArrived(qint64, const QVariantMap &)),
        SLOT(handleMessageArrived(qint64, const QVariantMap &)));
    connect(channel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
    connect(channel, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
    return true;
}

// Sends a 'history request' message to the server.
void QCServerChannel::sendHistoryRequest()
{
    QVariantMap msg;
    msg.insert("type", HistoryRequest);
    sendMessage(msg);
}

// Sends a message to the server.
void QCServerChannel::sendMessage(const QVariantMap &msg)
{
    if (!channel) {
        const char *emsg = "channel not connected";
        qWarning("WARNING: %s", emsg);
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
    if (!server_.listen(port)) {
        setLastError(QString("server_.listen() failed: %1").arg(server_.lastError()));
        return false;
    }
    QObject::connect(&server_, SIGNAL(channelConnected(QCChannel *)), SLOT(handleChannelConnected(QCChannel *)));
    return true;
}

// Sends a 'history' message to a specific qclserver client.
void QCClientChannels::sendHistory(const QStringList &h, qint64 qclserver)
{
    if (!channels_.contains(qclserver)) {
        qWarning("WARNING: QCClientChannels::sendHistory(): qclserver %lld no longer connected", qclserver);
        return;
    }

    QVariantMap msg;
    msg.insert("type", History);
    msg.insert("value", h);
    channels_.value(qclserver)->sendMessage(msg);
}

// Sends a message to all clients or to a specific client given in the field "client" (if set and >= 0).
void QCClientChannels::sendMessage(const QVariantMap &msg)
{
    bool ok;
    qint64 client = msg.value("client").toLongLong(&ok);
    if (ok && (client >= 0)) {
        if (!channels_.contains(client)) {
            qWarning("WARNING: QCClientChannels::sendMessages(): client %lld no longer connected", client);
            return;
        }
        channels_.value(client)->sendMessage(msg);
    } else  {
        foreach (QCChannel *channel, channels_.values())
            channel->sendMessage(msg);
    }
}

void QCClientChannels::handleChannelConnected(QCChannel *channel)
{
    qDebug() << "new client connected:" << channel->peerInfo().toLatin1().data();
    connect(
        channel, SIGNAL(messageArrived(qint64, const QVariantMap &)),
        SLOT(handleMessageArrived(qint64, const QVariantMap &)));
    connect(channel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
    connect(channel, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
    channels_.insert(channel->id(), channel);
    emit clientConnected(channel->id());
}

void QCClientChannels::handleChannelDisconnected()
{
    QCChannel *channel = static_cast<QCChannel *>(sender());
    qDebug() << "client disconnected:" << channel->peerInfo().toLatin1().data();
    channels_.remove(channel->id());
    channel->deleteLater(); // ### or 'delete channel' directly?
}
