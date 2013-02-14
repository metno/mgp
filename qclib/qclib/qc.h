#ifndef QC_H
#define QC_H

#include <QtCore> // ### TODO: include relevant headers only
#include <QtNetwork> // ### TODO: include relevant headers only
#include "qcglobal.h"

namespace qclib {

// This class handles basic transfer of QVariantMap messages between two peers.
class QCChannel : public QObject
{
    Q_OBJECT
public:
    virtual ~QCChannel() {}
    qint64 id() const { return id_; }
    void sendMessage(const QVariantMap &);
    QString lastError() const;
    virtual bool isConnected() const = 0;
protected:
    QCChannel(QIODevice * = 0);
    void initSocket();
    void setLastError(const QString &);
    QIODevice *socket_;
private:
    qint64 id_;
    static qint64 nextId_;
    QByteArray msgbuf_;
    QString lastError_;
signals:
    void error(const QString &);
    void socketDisconnected();
    void messageArrived(qint64, const QVariantMap &);
private slots:
    void readyRead();
};

class QCLocalChannel : public QCChannel
{
    Q_OBJECT
public:
    QCLocalChannel(QLocalSocket * = 0);
    virtual ~QCLocalChannel();
    bool connectToServer(const QString &);
    virtual bool isConnected() const;
private:
    void initSocket();
private slots:
    void handleSocketError(QLocalSocket::LocalSocketError);
};

class QCTcpChannel : public QCChannel
{
    Q_OBJECT
public:
    QCTcpChannel(QTcpSocket * = 0);
    virtual ~QCTcpChannel();
    QString peerInfo() const { return peerInfo_; }
    bool connectToServer(const QString &, const quint16);
    virtual bool isConnected() const;
private:
    QString peerInfo_;
    void initSocket();
private slots:
    void handleSocketError(QAbstractSocket::SocketError);
};

class QCChannelServer : public QObject
{
    Q_OBJECT
public:
    QCChannelServer();
    virtual ~QCChannelServer() {}
    QString lastError() const;
protected:
    void setLastError(const QString &);
    QString lastError_;
signals:
    void channelConnected(QCChannel *);
};

class QCLocalChannelServer : public QCChannelServer
{
    Q_OBJECT
public:
    QCLocalChannelServer();
    virtual ~QCLocalChannelServer();
    bool listen();
private:
    QLocalServer server_;
    QFileSystemWatcher fileSysWatcher_;
private slots:
    void newConnection();
signals:
    void serverFileChanged(const QString &);
};

class QCTcpChannelServer : public QCChannelServer
{
    Q_OBJECT
public:
    bool listen(quint16);
private:
    QTcpServer server_;
private slots:
    void newConnection();
};

} // namespace qclib

#endif // QC_H
