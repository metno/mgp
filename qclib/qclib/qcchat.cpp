#include "qcchat.h"
#include "qcglobal.h"

namespace qclib {

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
    : lastError_("<no last error string set yet>")
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

void QCBase::sendShowChatWindow()
{
    QVariantMap msg;
    msg.insert("type", ShowChatWin);
    sendMessage(msg);
}

void QCBase::sendHideChatWindow()
{
    QVariantMap msg;
    msg.insert("type", HideChatWin);
    sendMessage(msg);
}

static QString truncateText(const QString &text)
{
    const int maxSize = 1024;
    if (text.size() <= maxSize)
        return text;
    qWarning("WARNING: text too long (%d chars); truncating to %d chars", text.size(), maxSize);
    return text.left(maxSize);
}

void QCBase::sendChatMessage(const QString &text, const QString &user, int channelId, int timestamp)
{
    QVariantMap msg;
    msg.insert("type", ChatMsg);
    msg.insert("text", truncateText(text));
    msg.insert("user", user);
    msg.insert("channelId", channelId);
    msg.insert("timestamp", timestamp);
    sendMessage(msg);
}

void QCBase::sendNotification(const QString &text, const QString &user, int channelId, int timestamp)
{
    QVariantMap msg;
    msg.insert("type", Notification);
    msg.insert("text", truncateText(text));
    msg.insert("user", user);
    msg.insert("channelId", channelId);
    msg.insert("timestamp", timestamp);
    sendMessage(msg);
}

void QCBase::sendChannelSwitch(int channelId, const QString &user)
{
    QVariantMap msg;
    msg.insert("type", ChannelSwitch);
    msg.insert("channelId", channelId);
    msg.insert("user", user);
    sendMessage(msg);
}

void QCBase::sendFullNameChange(const QString &fullName, const QString &user)
{
    QVariantMap msg;
    msg.insert("type", FullNameChange);
    msg.insert("fullName", fullName);
    msg.insert("user", user);
    sendMessage(msg);
}

void QCBase::handleMessageArrived(qint64 peerId, const QVariantMap &msg)
{
    Q_ASSERT(!msg.contains("type"));
    bool ok;
    const MsgType type = static_cast<MsgType>(msg.value("type").toInt(&ok));
    if (!ok) {
        qDebug() << "msg:" << msg;
        qFatal("ERROR: failed to convert message type to int: %s", msg.value("type").toString().toLatin1().data());
    }
    if (type == Init) {
        emit init(msg, peerId);
    } else if (type == ShowChatWin) {
        emit showChatWindow();
    } else if (type == HideChatWin) {
        emit hideChatWindow();
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
    } else if (type == Users) {
        Q_ASSERT(msg.value("users").canConvert(QVariant::StringList));
        Q_ASSERT(msg.value("channelIds").canConvert(QVariant::VariantList));
        emit users(msg.value("users").toStringList(), toIntList(msg.value("channelIds").toList()));
    } else if (type == ChannelSwitch) {
        Q_ASSERT(msg.value("channelId").canConvert(QVariant::Int));
        Q_ASSERT(msg.value("user").canConvert(QVariant::String));
        emit channelSwitch(msg.value("channelId").toInt(), msg.value("user").toString(), peerId);
    } else if (type == FullNameChange) {
        Q_ASSERT(msg.value("fullName").canConvert(QVariant::String));
        Q_ASSERT(msg.value("user").canConvert(QVariant::String));
        emit fullNameChange(msg.value("fullName").toString(), msg.value("user").toString(), peerId);
    } else {
        qFatal("invalid message type: %d", type);
    }
}

void QCBase::handleChannelError(const QString &msg)
{
    qDebug() << "channel error:" << msg.toLatin1().data();
    lastError_ = msg;
}

QCServerChannel::QCServerChannel(QCChannel *channel)
    : channel_(channel)
{
}

bool QCServerChannel::isConnected() const
{
    return channel_ && channel_->isConnected();
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

// Sends an init message to the server.
void QCServerChannel::sendInit(const QVariantMap &msg_)
{
    QVariantMap msg(msg_);
    msg.insert("type", Init);
    sendMessage(msg);
}

void QCServerChannel::handleChannelDisconnected()
{
    Q_ASSERT(channel_);
    // qDebug() << "server disconnected:" << channel_->peerInfo().toLatin1().data();
    channel_->deleteLater();
    channel_ = 0;
    emit serverDisconnected();
}

QCLocalServerChannel::QCLocalServerChannel()
    : QCServerChannel(new QCLocalChannel)
{
}

static bool processExists(qint64 pid)
{
    QProcess process;
    process.start(QString("/bin/ps -p %1 -o pid=").arg(pid));
    if (!process.waitForFinished(-1))
        qFatal("ERROR: failed to run /bin/ps, error code: %d", process.error());
    bool ok;
    QString stdout = process.readAllStandardOutput();
    const qint64 pid_ = stdout.toLongLong(&ok);
    Q_ASSERT((!ok) || (pid_ == pid));
    Q_UNUSED(pid_);
    return ok;
}

// Connects to a qclserver, starting a new one if necessary. A new qclserver is passed
// \a chost and \a cport.
// Returns true iff the connection attempt was successful.
bool QCLocalServerChannel::connectToServer(const QString &chost, quint16 cport)
{
    if (isConnected()) {
        setLastError("channel already connected, please disconnect first");
        return false;
    }

    for (int mainIter = 0; mainIter < 2; ++mainIter) { // we should be able to reach a decision within 2 iterations

        bool localStart = false; // whether we started a qclserver ourselves

        QString serverPath;
        qint64 pid = 0;

        if (!localServerFileExists(&serverPath, &pid)) {
            // start a qclserver
            QString args = QString("--chost %1 --cport %2").arg(chost).arg(cport);
            const QString workingDir = QDir::homePath(); // ### FOR NOW
            qint64 startpid;
            if (!QProcess::startDetached("qclserver", args.split(" "), workingDir, &startpid)) {
                qWarning("WARNING: failed to start qclserver for user %s", qgetenv("USER").data());
                return false;
            } else {
                qDebug("started qclserver for user %s; pid = %lld", qgetenv("USER").data(), startpid);
            }

            localStart = true;

            // as long as the new qclserver is running, wait a while for the server file to appear
            const int ntries = 10; // timeout = 10 secs
            int i;
            for (i = 0; i < ntries; ++i) {
                if (!processExists(startpid)) {
                    const char *msg = "new qclserver terminated prematurely";
                    qWarning("WARNING: %s", msg);
                    setLastError(msg);
                    return false;
                }
                if (localServerFileExists(&serverPath, &pid)) {
                    Q_ASSERT(pid == startpid);
                    break;
                }
                sleep(1);
            }
            if (i == ntries) {
                const char *msg = "new qclserver failed to create server file";
                qWarning("WARNING: %s", msg);
                setLastError(msg);
                return false;
            }
        }

        Q_ASSERT(!serverPath.isEmpty());

        if (channel_)
            delete channel_;
        QCLocalChannel *lchannel = new QCLocalChannel;
        channel_ = lchannel;

        if (!lchannel->connectToServer(serverPath)) {
            if (localStart) {
                qWarning("WARNING: failed to connect to locally started qclserver");
                return false;
            }
            // at this point, we assume that the server file is obsolete and needs
            // to be removed before we try _once_ again
            qDebug() << "removing presumably obsolete server file:" << serverPath;
            if (!QLocalServer::removeServer(serverPath)) {
                qWarning("WARNING: failed to remove server file");
                return false;
            }
        } else {
            // successful connection
            connect(
                lchannel, SIGNAL(messageArrived(qint64, const QVariantMap &)),
                SLOT(handleMessageArrived(qint64, const QVariantMap &)));
            connect(lchannel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
            connect(lchannel, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
            return true;
        }
    }

    qWarning("WARNING: unable to connect to a qclserver within two main iterations");
    return false;
}

QCTcpServerChannel::QCTcpServerChannel()
    : QCServerChannel(new QCTcpChannel)
{
}

// Connects to a qccserver running at \a host and listening on \a port.
// Returns true iff the connection attempt was successful.
bool QCTcpServerChannel::connectToServer(const QString &host, const quint16 port)
{
    if (isConnected()) {
        setLastError("channel already connected, please disconnect first");
        return false;
    }

    if (channel_)
        delete channel_;
    QCTcpChannel * tchannel = new QCTcpChannel;
    channel_ = tchannel;

    if (!tchannel->connectToServer(host, port)) {
        setLastError(tchannel->lastError());
        return false;
    }
    connect(
        tchannel, SIGNAL(messageArrived(qint64, const QVariantMap &)),
        SLOT(handleMessageArrived(qint64, const QVariantMap &)));
    connect(tchannel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
    connect(tchannel, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
    return true;
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

// Sends an init message to the given client.
void QCClientChannels::sendInit(const QVariantMap &msg_, qint64 client)
{
    QVariantMap msg(msg_);
    msg.insert("type", Init);
    msg.insert("peer", client);
    sendMessage(msg);
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

// Disconnects a client channel.
void QCClientChannels::close(qint64 client)
{
    Q_ASSERT(channels_.contains(client));
    delete channels_.take(client);
}

// Sends a message to all clients or to a specific client given in the field "peer" (if set and >= 0).
void QCClientChannels::sendMessage(const QVariantMap &msg)
{
    bool ok;
    qint64 client = msg.value("peer").toLongLong(&ok);
    if (ok && (client >= 0)) {
        if (!channels_.contains(client)) {
            qWarning("WARNING: QCClientChannels::sendMessage(): client %lld no longer connected", client);
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
    emit clientDisconnected(id);
    channel->deleteLater(); // ### or 'delete channel' directly?
}

QCLocalClientChannels::QCLocalClientChannels()
{
    connect(&server_, SIGNAL(serverFileChanged(const QString &)), SIGNAL(serverFileChanged(const QString &)));
}

bool QCLocalClientChannels::listen()
{
    if (!server_.listen()) {
        setLastError(QString("QCLocalClientChannels::listen(): listen() failed: %1").arg(server_.lastError()));
        return false;
    }
    connect(&server_, SIGNAL(channelConnected(QCChannel *)), SLOT(handleChannelConnected(QCChannel *)));
    return true;
}

QCTcpClientChannels::QCTcpClientChannels()
{
}

bool QCTcpClientChannels::listen(const qint16 port)
{
    if (!server_.listen(port)) {
        setLastError(QString("QCTcpClientChannels::listen(): listen() failed: %1").arg(server_.lastError()));
        return false;
    }
    connect(&server_, SIGNAL(channelConnected(QCChannel *)), SLOT(handleChannelConnected(QCChannel *)));
    return true;
}

} // namespace qclib
