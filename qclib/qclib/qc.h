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
    bool connectToServer(const QString &, const quint16);
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

// This class is instantiated in the client to handle the server channel.
class QCServerChannel : public QObject
{
    Q_OBJECT
public:
    QCServerChannel();
    bool connectToServer(const QString &, const quint16);
    QString lastError() const;
public slots:
    void showChatWindow();
    void hideChatWindow();
    void sendNotification(const QString &);
private:
    QCChannel *channel;
    QString lastError_;
    void sendMessage(const QString &);
private slots:
    void handleMessageArrived(const QString &);
    void handleChannelError(const QString &);
    void handleChannelDisconnected();
signals:
    void serverDisconnected();
    void chatWindowShown();
    void chatWindowHidden();
    void notification(const QString &);
};

// This class is instantiated in the server to handle client channels.
class QCClientChannels : public QObject
{
    Q_OBJECT
public:
    QCClientChannels();
    bool listen(const qint16);
    void notifyChatWindowShown();
    void notifyChatWindowHidden();
    void sendNotification(const QString &);
    QString lastError() const;
private slots:
    void handleChannelConnected(QCChannel *);
    void handleMessageArrived(const QString &);
    void handleChannelError(const QString &);
    void handleChannelDisconnected();
private:
    QCChannelServer server;
    QList<QCChannel *> channels;
    QString lastError_;
    void broadcast(const QString &);
signals:
    void clientConnected();
    void showChatWindowRequested();
    void hideChatWindowRequested();
    void notificationRequested(const QString &);
};

#endif // QC_H
