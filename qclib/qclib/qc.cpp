#include "qc.h"

bool QCChannel::connect(const quint16 port) { Q_UNUSED(port); return false; }
void QCChannel::disconnect() { }
bool QCChannel::isConnected() const { return false; }
void QCChannel::sendMessage(const QString &msg) { Q_UNUSED(msg); }

bool QCChannelServer::listen(const qint16 port) { Q_UNUSED(port); return false; }
QString QCChannelServer::errorString() const { return "unimplemented"; }



/*   OLD CODE BELOW

void BMMessage::append(const QByteArray &data)
{
    Q_ASSERT(data.size() <= remaining());
    msg.append(data);
    if (isComplete()) {
        if (msg.size() == 4) {
            // XML data size complete, prepare reception of data itself:
            const int dataSize =
                qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(msg.data()));
            Q_ASSERT(dataSize > 0);
            totSize += dataSize;
        }
    }
}

BMChannel::BMChannel(QTcpSocket *socket)
    : socket(socket)
    , state(Idle)
    , syncMsgSize(-1)
    , asyncMsg(0)
    , deleteRequested(false)
    , deliveringMessage(false)
    , puttingMessage(false)
{
    connect(
        socket, SIGNAL(error(QAbstractSocket::SocketError)),
        SIGNAL(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), SIGNAL(socketDisconnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
    Q_ASSERT(!socket->peerAddress().isNull());
    Q_ASSERT(socket->peerPort() != 0);
    peerInfo_ = QString("%1:%2")
        .arg(QHostInfo::fromName(socket->peerAddress().toString()).hostName())
        .arg(socket->peerPort());
}

BMChannel::~BMChannel()
{
    // socket->close(); ?
    delete socket;
}

void BMChannel::safeDelete()
{
    if (state == Sync) {
        deleteRequested = true;
        eventLoop.exit(0);
    } else {
//        delete this;
        deleteLater();
    }
}

// Sends \a data on the channel, prepending the size as a qint32 value at the front.
void BMChannel::sendMessage(const QByteArray &data)
{
    QByteArray msg;
    msg.resize(4);
    qToBigEndian<quint32>(data.size(), reinterpret_cast<uchar *>(msg.data()));
    msg.append(data);
    putMessage(msg);
}

BMChannel * BMChannel::createClient(const QString &host, const quint16 port, QString *error)
{
    QTcpSocket *socket_ = new QTcpSocket;
    socket_->connectToHost(host, port);
    if (socket_->waitForConnected(-1)) {
        return new BMChannel(socket_);
    } else {
        *error = QString("BMChannel::createClient() failed: %1").arg(socket_->errorString());
        delete socket_;
        return 0;
    }
}

void BMChannel::putMessage(const QByteArray &msg)
{
    puttingMessage = true;
    socket->write(msg);
    socket->waitForBytesWritten(-1);
    puttingMessage = false;
    if (socket->bytesAvailable())
        QTimer::singleShot(0, this, SLOT(readyRead()));
}

// Synchronously receives a message of a specific size. Returns an empty message upon error.
QByteArray BMChannel::getMessage(int size)
{
    Q_ASSERT(size > 0);
    Q_ASSERT(state == Idle);

    state = Sync;
    syncMsg.clear();
    syncMsgSize = size;

    // accumulate data in a local event loop ...
    if (eventLoop.exec(QEventLoop::WaitForMoreEvents) != 0)
        syncMsg.clear();

    if (deleteRequested)
        deleteLater();

    state = Idle;
    return syncMsg;
}

void BMChannel::readyRead()
{
    // the following test is necessary because we also call this slot from a zero timer
    // (i.e. two separate calls may in theory be scheduled on behalf of the same data;
    // one from the zero timer and one from the network interface itself):
    if (socket->bytesAvailable() == 0)
        return;

    // we don't want to recurse while delivering messages either ...
    if (deliveringMessage)
        return;

    // ... nor while putting a message ...
    if (puttingMessage)
        return;

    const bool recheck = readSegment();

    if (recheck && socket->bytesAvailable())
        QTimer::singleShot(0, this, SLOT(readyRead()));
}

// Reads the next available segment. Returns true iff it makes sense to immediately check if
// new data has arrived (while executing this function).
bool BMChannel::readSegment()
{
    bool recheck = false;

    if (state == Sync) {
        Q_ASSERT(asyncMsg == 0);

        QByteArray segment = socket->read(syncMsgSize - syncMsg.size());

        if (segment.isEmpty()) {
            handleError("BMChannel::readyRead()/Sync: read() failed while reading segment");
        } else {
            syncMsg.append(segment);
            if (syncMsg.size() == syncMsgSize)
                eventLoop.exit(0);
        }

    } else if (state == Async) {
        Q_ASSERT(asyncMsg != 0);
        Q_ASSERT(asyncMsg->remaining() > 0);

        QByteArray segment = socket->read(asyncMsg->remaining());

        if (segment.isEmpty()) {
            handleError("BMChannel::readyRead()/Async: read() failed while reading segment");
        } else {
            asyncMsg->append(segment);

            if (asyncMsg->isComplete()) {

                deliveringMessage = true;
                asyncMsg->emitArrived();
                deliveringMessage = false;

                delete asyncMsg;
                asyncMsg = 0;
                state = Idle;
            }

            recheck = true;
        }

    } else {
        Q_ASSERT(state == Idle);
        Q_ASSERT(asyncMsg == 0);

        asyncMsg = new BMMessage;
        emit connectSignals(asyncMsg);

        state = Async;
        recheck = true;
    }

    return recheck;
}

void BMChannel::handleError(const QString &what)
{
    if (state == Sync)
        eventLoop.exit(1);
    state = Idle;
    emit error(what);
}


//--------------------------------------------------------------------------------------

BMConnection::BMConnection()
    : channel(0)
    , connected(false)
{
}

BMConnection::BMConnection(BMChannel *channel)
    : channel(channel)
    , connected(false)
{
    connectChannel();
}

BMConnection::~BMConnection()
{
    if (channel)
        channel->safeDelete();
}

QString BMConnection::lastError() const
{
    return lastError_;
}

void BMConnection::setLastError(const QString &lastError_)
{
    this->lastError_ = lastError_;
}

void BMConnection::connectChannel()
{
    if (!channel)
        return;

#ifdef BMDEBUG
    qDebug() << "new connection; peer info:" << channel->peerInfo();
#endif

    connect(channel, SIGNAL(error(const QString &)), SLOT(handleError(const QString &)));
    connect(channel, SIGNAL(error(const QString &)), SIGNAL(error(const QString &)));
    connect(
        channel, SIGNAL(socketError(QAbstractSocket::SocketError)),
        SLOT(handleSocketError(QAbstractSocket::SocketError)));
    connect(channel, SIGNAL(socketDisconnected()), SLOT(handleSocketDisconnected()));
    connect(channel, SIGNAL(connectSignals(const BMMessage *)),
            SLOT(connectSignals(const BMMessage *)));

    connected = true;
}

void BMConnection::deleteChannel()
{
    static bool deletingChannel = false;
    if (deletingChannel || !channel)
        return;
    deletingChannel = true;
#ifdef BMDEBUG
    qDebug() << "deleting channel; peer info:" << channel->peerInfo();
#endif
    channel->safeDelete();
    channel = 0;
    deletingChannel = false;
}

void BMConnection::handleError(const QString &what)
{
#ifdef BMDEBUG
    qDebug() << "BMConnection::error():" << what;
#else
    Q_UNUSED(what);
#endif
    deleteChannel();
    connected = false;
}

void BMConnection::handleSocketError(QAbstractSocket::SocketError error)
{
#ifdef BMDEBUG
    qDebug() << "BMConnection::socketError():" << error;
#else
    Q_UNUSED(error);
#endif
    deleteChannel();
    connected = false;
}

void BMConnection::handleSocketDisconnected()
{
#ifdef BMDEBUG
    qDebug() << "BMConnection::socketDisconnected()";
#endif
    emit disconnected();
    deleteChannel();
    connected = false;
}

void BMConnection::connectSignals(const BMMessage *msg)
{
    connect(msg, SIGNAL(arrived(const QByteArray &)), SLOT(msgArrived(const QByteArray &)));
}

BMClientConnection::BMClientConnection(
    const QString &host, quint16 port, BMRequest::OutputFormat outputFormat,
    const QStringList &args)
    : host(host)
    , port(port)
    , outputFormat(outputFormat)
    , args(args)
{
}

// Attempts to connect to the server. Returns true iff a connection was successfully established.
bool BMClientConnection::connect()
{
    QString error;
    channel = BMChannel::createClient(host, port, &error);
    if (!channel) {
        setLastError(error);
        return false;
    }
    connectChannel();
    return isConnected();
}

bool BMClientConnection::sendRequest(BMRequest *request)
{
    QString error;
    QByteArray buf = request->toRequestBuffer(&error);
    delete request;
    if (buf.isEmpty()) {
        setLastError(error);
        return false;
    }
    channel->sendMessage(buf);
    return true;
}

// Handles a reply message from the server.
void BMClientConnection::msgArrived(const QByteArray &data)
{
    BMRequest *request = BMRequest::create(data);
    Q_ASSERT(request);
    request->handleReply(outputFormat, args);
    delete request;
    emit replyDone();
}

BMServerConnection::BMServerConnection(BMChannel *channel, QSqlDatabase *database)
    : BMConnection(channel)
    , database(database)
{
}

// Handles a request message from the client.
void BMServerConnection::msgArrived(const QByteArray &data)
{
    BMRequest *request = BMRequest::create(data, database);
    Q_ASSERT(request);
    channel->sendMessage(request->toReplyBuffer());
    delete request;
}

BMServer::BMServer(const quint16 port, QSqlDatabase *database)
    : valid(false)
    , database(database)
{
    if (!server.listen(QHostAddress::Any, port)) {
#ifdef BMDEBUG
        qDebug() << "BMServer::BMServer(): listen() failed:" << server.errorString();
#endif
        return;
    }
    connect(&server, SIGNAL(newConnection()), SLOT(newConnection()));
#ifdef BMDEBUG
    qDebug() << "accepting client connections on port" << port << "...";
#endif
    valid = true;
}

void BMServer::newConnection()
{
    BMChannel *channel = new BMChannel(server.nextPendingConnection());
    BMServerConnection *connection = new BMServerConnection(channel, database);
#ifdef BMDEBUG
    if (!connection->isConnected())
        qDebug() << "BMServer::newConnection(): new connection failed to initialize";
#else
    Q_UNUSED(connection);
#endif
}

*/
