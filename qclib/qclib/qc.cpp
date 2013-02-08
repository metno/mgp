#include "qc.h"
#include "qcglobal.h"

namespace qclib {

static QString createLocalServerPath()
{
    return QString("/tmp/qclserver_%1_%2").arg(qgetenv("USER").data()).arg(qApp->applicationPid());
}

QCChannel::QCChannel(QIODevice *socket)
    : socket_(socket)
    , id_(nextId_++)
    , lastError_("<no last error string set yet>")
{
}

void QCChannel::initSocket()
{
    Q_ASSERT(socket_);
    connect(socket_, SIGNAL(disconnected()), SIGNAL(socketDisconnected()));
    connect(socket_, SIGNAL(readyRead()), SLOT(readyRead()));
}

QString QCChannel::lastError() const
{
    return lastError_;
}

void QCChannel::setLastError(const QString &lastError_)
{
    this->lastError_ = lastError_;
}

// Sends the variant map \a msg as a message on the channel.
void QCChannel::sendMessage(const QVariantMap &msg)
{
    if (!isConnected()) {
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

        // qDebug() << "msg arrived:" << msg;
        emit messageArrived(id(), msg);
    }
}

qint64 QCChannel::nextId_ = 0;

QCLocalChannel::QCLocalChannel(QLocalSocket * socket)
    : QCChannel(socket)
{
    if (socket_)
        initSocket();
}

QCLocalChannel::~QCLocalChannel()
{
    if (socket_)
        delete socket_;
}

bool QCLocalChannel::connectToServer(const QString &serverPath)
{
    if (socket_) {
        setLastError("socket already connected, please disconnect first");
        return false;
    }

    QLocalSocket *lsocket = new QLocalSocket;
    socket_ = lsocket;
    lsocket->connectToServer(serverPath);
    if (!lsocket->waitForConnected(-1)) {
        setLastError(QString("waitForConnected() failed: %1").arg(socket_->errorString()));
        delete socket_;
        socket_ = 0;
        return false;
    }

    initSocket();
    return true;
}

void QCLocalChannel::initSocket()
{
    QLocalSocket *lsocket = qobject_cast<QLocalSocket *>(socket_);
    Q_ASSERT(lsocket);
    connect(
        lsocket, SIGNAL(error(QLocalSocket::LocalSocketError)),
        SLOT(handleSocketError(QLocalSocket::LocalSocketError)));
    QCChannel::initSocket();
}

bool QCLocalChannel::isConnected() const
{
    return socket_ != 0;
}

void QCLocalChannel::handleSocketError(QLocalSocket::LocalSocketError e)
{
    if (e != QLocalSocket::PeerClosedError) // for now, don't consider this an error
        emit error(socket_->errorString());
}

QCTcpChannel::QCTcpChannel(QTcpSocket *socket)
    : QCChannel(socket)
{
    if (socket_)
        initSocket();
}

QCTcpChannel::~QCTcpChannel()
{
    if (socket_)
        delete socket_;
}

// Connects the channel to a server on \a host listening on \a port.
bool QCTcpChannel::connectToServer(const QString &host, const quint16 port)
{
    if (socket_) {
        setLastError("socket already connected, please disconnect first");
        return false;
    }

    QTcpSocket *tsocket = new QTcpSocket;
    socket_ = tsocket;
    tsocket->connectToHost(host, port);
    if (!tsocket->waitForConnected(-1)) {
        setLastError(QString("waitForConnected() failed: %1").arg(socket_->errorString()));
        delete socket_;
        socket_ = 0;
        return false;
    }

    initSocket();
    return true;
}

void QCTcpChannel::initSocket()
{
    QTcpSocket *tsocket = qobject_cast<QTcpSocket *>(socket_);
    Q_ASSERT(tsocket);
    connect(
        tsocket, SIGNAL(error(QAbstractSocket::SocketError)),
        SLOT(handleSocketError(QAbstractSocket::SocketError)));
    QCChannel::initSocket();
    Q_ASSERT(!tsocket->peerAddress().isNull());
    Q_ASSERT(tsocket->peerPort() != 0);
    peerInfo_ = QString("%1:%2")
        .arg(QHostInfo::fromName(tsocket->peerAddress().toString()).hostName())
        .arg(tsocket->peerPort());
}

bool QCTcpChannel::isConnected() const
{
    return socket_ != 0;
}

void QCTcpChannel::handleSocketError(QAbstractSocket::SocketError e)
{
    if (e != QAbstractSocket::RemoteHostClosedError) // for now, don't consider this an error
        emit error(socket_->errorString());
}

QCChannelServer::QCChannelServer()
    : lastError_("<no last error string set yet>")
{
}

QString QCChannelServer::lastError() const
{
    return lastError_;
}

void QCChannelServer::setLastError(const QString &lastError_)
{
    this->lastError_ = lastError_;
}

QCLocalChannelServer::QCLocalChannelServer()
{
    connect(&fileSysWatcher_, SIGNAL(fileChanged(const QString &)), SIGNAL(serverFileChanged(const QString &)));
}

QCLocalChannelServer::~QCLocalChannelServer()
{
    const QString serverPath = server_.fullServerName();
    if (!QLocalServer::removeServer(serverPath))
        qWarning("WARNING: failed to remove server file upon cleanup: %s", serverPath.toLatin1().data());
    else
        qDebug() << "removed server file:" << serverPath;
}

bool QCLocalChannelServer::listen()
{
    QString serverPath;

    if (localServerFileExists(&serverPath)) {
        setLastError(QString("QCLocalChannelServer::listen(): server file already exists: %1").arg(serverPath));
        return false;
    }

    if (!fileSysWatcher_.files().empty()) {
        setLastError(
            QString("QCLocalChannelServer::listen(): file system watcher already watching %1 other server file(s): %2 ...")
            .arg(fileSysWatcher_.files().size()).arg(fileSysWatcher_.files().first()));
        return false;
    }

    serverPath = createLocalServerPath();

    if (!server_.listen(serverPath)) {
        setLastError(QString("QCLocalChannelServer::listen(): listen() failed: %1").arg(server_.errorString()));
        return false;
    }

    fileSysWatcher_.addPath(serverPath);
    connect(&server_, SIGNAL(newConnection()), SLOT(newConnection()));
    // qDebug() << "accepting client connections on path" << serverPath << "...";

    return true;
}

void QCLocalChannelServer::newConnection()
{
    emit channelConnected(new QCLocalChannel(server_.nextPendingConnection()));
}

bool QCTcpChannelServer::listen(quint16 port)
{
    if (!server_.listen(QHostAddress::Any, port)) {
        setLastError(QString("QCTcpChannelServer::listen(): listen() failed: %1").arg(server_.errorString()));
        return false;
    }
    connect(&server_, SIGNAL(newConnection()), SLOT(newConnection()));
    // qDebug() << "accepting client connections on port" << port << "...";
    return true;
}

void QCTcpChannelServer::newConnection()
{
    emit channelConnected(new QCTcpChannel(server_.nextPendingConnection()));
}

} // namespace qclib
