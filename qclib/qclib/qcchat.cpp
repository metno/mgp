#include "qcchat.h"

static QList<int> toIntList(const QList<QVariant> &vlist)
{
    QList<int> ilist;
    bool ok;
    foreach (QVariant v, vlist) {
        const int i = v.toInt(&ok);
        Q_ASSERT(ok);
        ilist.append(i);
    }
    return ilist;
}

static QList<QVariant> toVariantList(const QList<int> &ilist)
{
    QList<QVariant> vlist;
    foreach (int i, ilist)
        vlist.append(i);
    return vlist;
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

static QString truncateText(const QString &text)
{
    const int maxSize = 1024;
    if (text.size() <= maxSize)
        return text;
    qWarning("text too long (%d chars); truncating to %d chars", text.size(), maxSize);
    return text.left(maxSize);
}

void QCBase::sendChatMessage(const QString &text, const QString &user, int channelId, int timestamp)
{
    QVariantMap msg;
    msg.insert("text", truncateText(text));
    msg.insert("user", user);
    msg.insert("channelId", channelId);
    msg.insert("timestamp", timestamp);
    msg.insert("type", ChatMsg);
    sendMessage(msg);
}

void QCBase::sendNotification(const QString &text, const QString &user, int channelId, int timestamp)
{
    QVariantMap msg;
    msg.insert("text", truncateText(text));
    msg.insert("user", user);
    msg.insert("channelId", channelId);
    msg.insert("timestamp", timestamp);
    msg.insert("type", Notification);
    sendMessage(msg);
}

void QCBase::sendChannelSwitch(int channelId, const QString &user)
{
    QVariantMap msg;
    msg.insert("channelId", channelId);
    msg.insert("user", user);
    msg.insert("type", ChannelSwitch);
    sendMessage(msg);
}

void QCBase::handleMessageArrived(qint64 peerId, const QVariantMap &msg)
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
        Q_ASSERT(msg.value("channelId").canConvert(QVariant::Int));
        Q_ASSERT(msg.value("timestamp").canConvert(QVariant::Int));
        emit chatMessage(
            msg.value("text").toString(), msg.value("user").toString(), msg.value("channelId").toInt(),
            msg.value("timestamp").toInt());
    } else if (type == Notification) {
        Q_ASSERT(msg.value("text").canConvert(QVariant::String));
        Q_ASSERT(msg.value("user").canConvert(QVariant::String));
        Q_ASSERT(msg.value("channelId").canConvert(QVariant::Int));
        Q_ASSERT(msg.value("timestamp").canConvert(QVariant::Int));
        emit notification(
            msg.value("text").toString(), msg.value("user").toString(), msg.value("channelId").toInt(),
            msg.value("timestamp").toInt());
    } else if (type == Initialization) {
        emit initialization(peerId, msg);
    } else if (type == Channels) {
        Q_ASSERT(msg.value("channels").canConvert(QVariant::StringList));
        emit channels(msg.value("channels").toStringList());
    } else if (type == History) {
        Q_ASSERT(msg.value("history").canConvert(QVariant::StringList));
        emit history(msg.value("history").toStringList());
    } else if (type == Users) {
        Q_ASSERT(msg.value("users").canConvert(QVariant::StringList));
        Q_ASSERT(msg.value("channelIds").canConvert(QVariant::VariantList));
        emit users(msg.value("users").toStringList(), toIntList(msg.value("channelIds").toList()));
    } else if (type == ChannelSwitch) {
        Q_ASSERT(msg.value("channelId").canConvert(QVariant::Int));
        Q_ASSERT(msg.value("user").canConvert(QVariant::String));
        emit channelSwitch(peerId, msg.value("channelId").toInt(), msg.value("user").toString());
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
    : channel_(0)
{
}

bool QCServerChannel::connectToServer(const QString &host, const quint16 port)
{
    if (isConnected()) {
        setLastError("channel already connected, please disconnect first");
        return false;
    }

    if (channel_)
        delete channel_;
    channel_ = new QCChannel;

    if (!channel_->connectToServer(host, port)) {
        setLastError(channel_->lastError());
        return false;
    }
    connect(
        channel_, SIGNAL(messageArrived(qint64, const QVariantMap &)),
        SLOT(handleMessageArrived(qint64, const QVariantMap &)));
    connect(channel_, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
    connect(channel_, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
    return true;
}

bool QCServerChannel::isConnected() const
{
    return channel_ && channel_->isConnected();
}

// Initializes communication.
void QCServerChannel::initialize(const QVariantMap &msg_)
{
    QVariantMap msg(msg_);
    msg.insert("type", Initialization);
    sendMessage(msg);
}

// Sends a message to the server.
void QCServerChannel::sendMessage(const QVariantMap &msg)
{
    if (!isConnected()) {
        const char *emsg = "channel not connected";
        qWarning("WARNING: %s", emsg);
        setLastError(emsg);
        return;
    }
    channel_->sendMessage(msg);
}

void QCServerChannel::handleChannelDisconnected()
{
    Q_ASSERT(channel_);
    // qDebug() << "server disconnected:" << channel_->peerInfo().toLatin1().data();
    channel_->deleteLater();
    channel_ = 0;
    emit serverDisconnected();
}

QCClientChannels::QCClientChannels()
{
}

QCClientChannels::~QCClientChannels()
{
    // hm ... maybe avoid this loop by wrapping each channel pointer in a suitable smart pointer?
    foreach (QCChannel *channel, channels_.values())
        delete channel;
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

// Disconnects a client channel.
void QCClientChannels::close(qint64 qclserver)
{
    Q_ASSERT(channels_.contains(qclserver));
    delete channels_.take(qclserver);
}

// Sends a 'channels' message to a specific qclserver client.
void QCClientChannels::sendChannels(const QStringList &c, qint64 qclserver)
{
    if (!channels_.contains(qclserver)) {
        qWarning("WARNING: QCClientChannels::sendChannels(): qclserver %lld no longer connected", qclserver);
        return;
    }

    QVariantMap msg;
    msg.insert("type", Channels);
    msg.insert("channels", c);
    channels_.value(qclserver)->sendMessage(msg);
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
    msg.insert("history", h);
    channels_.value(qclserver)->sendMessage(msg);
}

// Sends a 'users' message to all clients.
void QCClientChannels::sendUsers(const QStringList &users, const QList<int> &channelIds)
{
    QVariantMap msg;
    msg.insert("type", Users);
    msg.insert("users", users);
    msg.insert("channelIds", toVariantList(channelIds));
    sendMessage(msg);
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
    // qDebug() << "new client connected:" << channel->peerInfo().toLatin1().data();
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
    // qDebug() << "client disconnected:" << channel->peerInfo().toLatin1().data();
    const qint64 id = channel->id();
    channels_.remove(id);
    channel->deleteLater(); // ### or 'delete channel' directly?
    emit clientDisconnected(id);
}
