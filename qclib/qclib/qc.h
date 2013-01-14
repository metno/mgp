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
    void sendMessage(const QVariantMap &);
    QString lastError() const;
private:
    QByteArray msgbuf_;
    QTcpSocket *socket;
    QString peerInfo_;
    QString lastError_;
    void initSocket();
    void setLastError(const QString &);
signals:
    void error(const QString &);
    void socketDisconnected();
    void messageArrived(const QVariantMap &);
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

class QCBase : public QObject
{
    Q_OBJECT
public:
    QString lastError() const;
protected:
    QCBase();
    enum MsgType { ShowChatWin, HideChatWin, ChatMsg, Notification };
    void setLastError(const QString &);
private:
    QString lastError_;
    virtual void sendMessage(const QVariantMap &) = 0;
public slots:
    void showChatWindow();
    void hideChatWindow();
    void sendChatMessage(const QString &);
    void sendNotification(const QString &);
protected slots:
    void handleMessageArrived(const QVariantMap &);
    void handleChannelError(const QString &);
signals:
    void chatWindowShown();
    void chatWindowHidden();
    void chatMessage(const QString &);
    void notification(const QString &);
};

// This class is instantiated in the client to handle the server channel.
class QCServerChannel : public QCBase
{
    Q_OBJECT
public:
    QCServerChannel();
    bool connectToServer(const QString &, const quint16);
private:
    QCChannel *channel;
    virtual void sendMessage(const QVariantMap &);
private slots:
    void handleChannelDisconnected();
signals:
    void serverDisconnected();
};

// This class is instantiated in the server to handle client channels.
class QCClientChannels : public QCBase
{
    Q_OBJECT
public:
    QCClientChannels();
    bool listen(const qint16);
private:
    QCChannelServer server;
    QList<QCChannel *> channels;
    virtual void sendMessage(const QVariantMap &);
private slots:
    void handleChannelConnected(QCChannel *);
    void handleChannelDisconnected();
signals:
    void clientConnected();
};

#endif // QC_H
