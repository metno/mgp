#ifndef QC_H
#define QC_H

#include <QtCore> // ### TODO: include relevant headers only
#include <QtNetwork> // ### TODO: include relevant headers only

class QCChannel : public QObject
{
    Q_OBJECT
public:
    bool connect(const quint16 port); // n/a for channels created by QCChannelServer
    void disconnect();
    bool isConnected() const;
    void sendMessage(const QString &msg);
signals:
    void messageArrived(const QString &);
    void error(const QString &);
    void disconnected();
};

class QCChannelServer : public QObject
{
    Q_OBJECT
public:
    bool listen(const qint16 port);
    QString errorString() const;
signals:
    void channelConnected(QCChannel *);
};




/*   OLD CODE BELOW

class BMMessage : public QObject
{
    Q_OBJECT
public:
    BMMessage() : totSize(4) {} // initially, we expect only XML data size (4 bytes)
    virtual ~BMMessage() {}
    int remaining() const { return totSize - msg.size(); }
    void append(const QByteArray &);
    bool isComplete() const { return remaining() == 0; }
    void emitArrived() const
    {
        emit arrived(QByteArray::fromRawData(msg.data() + 4, msg.size() - 4));
    }

private:
    int totSize; // total message size currently expected (including XML data size)
    QByteArray msg; // message buffer being read from the network

signals:
    // emits the XML part of the message as soon as the complete message has arrived:
    void arrived(const QByteArray &) const;
};

// ### Warning: This class is not reentrant!
class BMChannel : public QObject
{
    Q_OBJECT
public:
    BMChannel(QTcpSocket *);
    virtual ~BMChannel();
    void safeDelete();
    QString peerInfo() const { return peerInfo_; }
    void sendMessage(const QByteArray &);
    static BMChannel *createClient(const QString &host, const quint16 port, QString *error);

private:
    QTcpSocket *socket;
    enum {Idle, Async, Sync} state;
    int syncMsgSize;
    QByteArray syncMsg;
    BMMessage *asyncMsg;
    bool deleteRequested;
    bool deliveringMessage;
    bool puttingMessage;
    QEventLoop eventLoop;
    QString peerInfo_;

    void putMessage(const QByteArray &);
    QByteArray getMessage(int size);
    bool readSegment();

    void handleError(const QString &);

signals:
    void error(const QString &);
    void socketError(QAbstractSocket::SocketError);
    void socketDisconnected();
    void connectSignals(const BMMessage *);

private slots:
    void readyRead();
};


//-----------------------------------------------------------------------------

class BMConnection : public QObject
{
    Q_OBJECT
public:
    BMConnection();
    BMConnection(BMChannel *channel);
    virtual ~BMConnection();
    bool isConnected() const { return connected; }
    QString lastError() const;
protected:
    BMChannel *channel;
    void connectChannel();
    void setLastError(const QString &);
private:
    bool connected;
    QString lastError_;
    void deleteChannel();
private slots:
    void handleError(const QString &);
    void handleSocketError(QAbstractSocket::SocketError);
    void handleSocketDisconnected();
    void connectSignals(const BMMessage *);
    virtual void msgArrived(const QByteArray &) = 0;
signals:
    void disconnected();
    void error(const QString &);
};

class BMRequest;

class BMClientConnection : public BMConnection
{
    Q_OBJECT
public:
    BMClientConnection(
        const QString &host, const quint16 port, BMRequest::OutputFormat outputFormat,
        const QStringList &args);
    virtual ~BMClientConnection() {}
    bool connect();
    bool sendRequest(BMRequest *request);
private slots:
    void msgArrived(const QByteArray &);
private:
    QString host;
    quint16 port;
    BMRequest::OutputFormat outputFormat;
    QStringList args;

signals:
    void replyDone();
};

class QSqlDatabase;

class BMServerConnection : public BMConnection
{
    Q_OBJECT
public:
    BMServerConnection(BMChannel *channel, QSqlDatabase *database);
    virtual ~BMServerConnection() {}

private:
    QSqlDatabase *database;

private slots:
    void msgArrived(const QByteArray &);
};

class BMServer : public QObject
{
    Q_OBJECT
public:
    BMServer(const quint16 port, QSqlDatabase *database);
    virtual ~BMServer() {}
    bool isValid() const { return valid; }
private:
    QTcpServer server;
    bool valid;
    QSqlDatabase *database;
private slots:
    void newConnection();
};

*/

#endif // QC_H
