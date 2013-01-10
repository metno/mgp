#ifndef QC_H
#define QC_H

#include <QtCore> // ### TODO: include relevant headers only
#include <QtNetwork> // ### TODO: include relevant headers only

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
    QString peerInfo_;
    QString lastError_;
    void initSocket();
    void setLastError(const QString &);
signals:
    void error(const QString &);
    void socketDisconnected();
    void messageArrived(const QString &);

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
    void sendChatMessage(const QString &);
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
    void chatMessage(const QString &);
    void notification(const QString &);
};

// This class is instantiated in the server to handle client channels.
class QCClientChannels : public QObject
{
    Q_OBJECT
public:
    QCClientChannels();
    bool listen(const qint16);
    void showChatWindow();
    void hideChatWindow();
    void sendChatMessage(const QString &);
    void sendNotification(const QString &);
    QString lastError() const;
private:
    QCChannelServer server;
    QList<QCChannel *> channels;
    QString lastError_;
    void sendMessage(const QString &);
private slots:
    void handleChannelConnected(QCChannel *);
    void handleMessageArrived(const QString &);
    void handleChannelError(const QString &);
    void handleChannelDisconnected();
signals:
    void clientConnected();
    void chatWindowShown();
    void chatWindowHidden();
    void chatMessage(const QString &);
    void notification(const QString &);
};

#endif // QC_H
