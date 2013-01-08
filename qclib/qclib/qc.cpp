#include "qc.h"

class QCMessage : public QObject
{
    Q_OBJECT
public:
    QCMessage() : totSize(4) {} // initially we expect only payload size (4 bytes)
    virtual ~QCMessage() {}
    int remaining() const { return totSize - msg.size(); }
    void append(const QByteArray &);
    bool isComplete() const { return remaining() == 0; }
    void emitArrived() const
    {
        emit arrived(QString::fromUtf8(QByteArray::fromRawData(msg.data() + 4, msg.size() - 4).data()));
    }

private:
    int totSize; // total message size currently expected (including payload size)
    QByteArray msg; // message buffer being read from the network

signals:
    // emits the payload converted from UTF-8 as soon as the complete message has arrived:
    void arrived(const QString &) const;
};

void QCMessage::append(const QByteArray &data)
{
    Q_ASSERT(data.size() <= remaining());
    msg.append(data);
    if (isComplete()) {
        if (msg.size() == 4) {
            // payload size complete, prepare reception of data part:
            const int dataSize =
                qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(msg.data()));
            Q_ASSERT(dataSize > 0);
            totSize += dataSize;
        }
    }
}

QCChannel::QCChannel(QTcpSocket *socket)
    : socket(socket)
    , state(Idle)
    , msg(0)
    , deliveringMessage(false)
    , puttingMessage(false)
    , lastError_("<INITIAL VALUE (no error)>")
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
    QByteArray data = s.toUtf8();
    QByteArray msg;

    // insert the payload size as the message header
    msg.resize(4);
    qToBigEndian<quint32>(data.size(), reinterpret_cast<uchar *>(msg.data()));

    // append the payload itself
    msg.append(data);

    putMessage(msg);
}

void QCChannel::putMessage(const QByteArray &msg)
{
    puttingMessage = true;
    socket->write(msg);
    socket->waitForBytesWritten(-1);
    puttingMessage = false;
    if (socket->bytesAvailable())
        QTimer::singleShot(0, this, SLOT(readyRead()));
}

void QCChannel::readyRead()
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

void QCChannel::resetState()
{
    if (msg)
        delete msg;
    msg = 0;
    state = Idle;
}

// Reads the next available segment. Returns true iff it makes sense to immediately check if
// new data has arrived (while executing this function).
bool QCChannel::readSegment()
{
    bool recheck = false;

    if (state == Receiving) {
        Q_ASSERT(msg != 0);
        Q_ASSERT(msg->remaining() > 0);

        QByteArray segment = socket->read(msg->remaining());

        if (segment.isEmpty()) {
            handleError("QCChannel::readSegment(): read() failed");
        } else {
            msg->append(segment);

            if (msg->isComplete()) {

                deliveringMessage = true;
                msg->emitArrived();
                deliveringMessage = false;

                resetState();
            }

            recheck = true;
        }

    } else {
        Q_ASSERT(state == Idle);
        Q_ASSERT(msg == 0);

        msg = new QCMessage;
        connect(msg, SIGNAL(arrived(const QString &)), SIGNAL(messageArrived(const QString &)));

        state = Receiving;
        recheck = true;
    }

    return recheck;
}

void QCChannel::handleError(const QString &what)
{
    resetState();
    emit error(what);
}

void QCChannel::handleSocketError(QAbstractSocket::SocketError)
{
    resetState();
    emit error(socket->errorString());
}

QCChannelServer::QCChannelServer()
    : lastError_("<INITIAL VALUE (no error)>")
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

#include "qc.moc"
