#include "qc.h"

QMap<QString, QString> getOptions(const QStringList &args)
{
    QMap <QString, QString> options;
    for (int i = 0; i < args.size(); ++i)
        if ((args.at(i).indexOf("--") == 0) && (args.at(i).size() > 2))
            options.insert(args.at(i).mid(2), (i < (args.size() - 1)) ? args.at(i + 1) : QString());
    return options;
}

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
        qDebug() << "WARNING: readyRead() called with no bytes available";
        return;
    }

    msgbuf_.append(socket_->read(socket_->bytesAvailable()));

    while (true) {
        const int nlPos = msgbuf_.indexOf('\n');
        if (nlPos == -1)
            return; // no complete message in buffer at this point

        // extract first message from buffer
        QByteArray ba = QByteArray::fromBase64(msgbuf_.left(nlPos)); // decode
        msgbuf_ = msgbuf_.mid(nlPos + 1);

        // deserialize into variant map
        QDataStream dstream(ba);
        QVariantMap msg;
        dstream >> msg;

        //qDebug() << "msg arrived:" << msg;
        emit messageArrived(id(), msg);
    }
}

void QCChannel::handleSocketError(QAbstractSocket::SocketError e)
{
    if (e != QAbstractSocket::RemoteHostClosedError) // for now, don't consider this an error
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
    // qDebug() << "accepting client connections on port" << port << "...";
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
