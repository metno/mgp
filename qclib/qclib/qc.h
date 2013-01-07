#ifndef QC_H
#define QC_H

#include <QtCore> // ### TODO: include relevant headers only
#include <QtNetwork> // ### TODO: include relevant headers only

class QCMessage;

// ### Warning: This class is not reentrant!
class QCChannel : public QObject
{
    Q_OBJECT
public:
    QCChannel(QTcpSocket * = 0);
    virtual ~QCChannel();
    QString peerInfo() const { return peerInfo_; }
    bool connectToServer(const quint16 port);
    //void disconnect() { }; ### needed?
    bool isConnected() const;
    void sendMessage(const QString &);
    QString lastError() const;
private:
    QTcpSocket *socket;
    enum {Idle, Receiving} state;
    QCMessage *msg;
    bool deliveringMessage;
    bool puttingMessage;
    QString peerInfo_;
    QString lastError_;
    void initSocket();
    void putMessage(const QByteArray &);
    void resetState();
    bool readSegment();
    void handleError(const QString &);
    void setLastError(const QString &);
signals:
    void error(const QString &);
    void socketDisconnected(); // ### handle this signal too in client (rename from 'disconnected()')
    void messageArrived(const QString &);
    //void connectSignals(const BMMessage *); OBSOLETE?

private slots:
    void readyRead();
    void handleSocketError(QAbstractSocket::SocketError);
};

class QCChannelServer : public QObject
{
    Q_OBJECT
public:
    QCChannelServer();
    bool listen(const qint16 port);
    QString lastError() const;
private:
    QTcpServer server;
    void setLastError(const QString &);
    QString lastError_;
private slots:
    void newConnection();
signals:
    void channelConnected(QCChannel *);
};

#endif // QC_H
