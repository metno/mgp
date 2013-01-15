#ifndef QC_H
#define QC_H

#include <QtCore> // ### TODO: include relevant headers only
#include <QtNetwork> // ### TODO: include relevant headers only

// Chat event types
#define CHATMESSAGE 0
#define NOTIFICATION 1

// ### Warning: This class is not reentrant!
class QCChannel : public QObject
{
    Q_OBJECT
public:
    QCChannel(QTcpSocket * = 0);
    virtual ~QCChannel();
    qint64 id() const { return id_; }
    QString peerInfo() const { return peerInfo_; }
    bool connectToServer(const QString &, const quint16);
    //void disconnect() { }; ### needed?
    bool isConnected() const;
    void sendMessage(const QVariantMap &);
    QString lastError() const;
private:
    qint64 id_;
    static qint64 nextId_;
    QByteArray msgbuf_;
    QTcpSocket *socket_;
    QString peerInfo_;
    QString lastError_;
    void initSocket();
    void setLastError(const QString &);
signals:
    void error(const QString &);
    void socketDisconnected();
    void messageArrived(qint64, const QVariantMap &);
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
    QTcpServer server_;
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
    enum MsgType { ShowChatWin, HideChatWin, ChatMsg, Notification, HistoryRequest, History };
    void setLastError(const QString &);
private:
    QString lastError_;
    virtual void sendMessage(const QVariantMap &) = 0;
public slots:
    void showChatWindow(qint64 = -1);
    void hideChatWindow(qint64 = -1);
    void sendChatMessage(const QString &, int timestamp = -1);
    void sendNotification(const QString &, int timestamp = -1);
protected slots:
    void handleMessageArrived(qint64, const QVariantMap &);
    void handleChannelError(const QString &);
signals:
    void chatWindowShown();
    void chatWindowHidden();
    void chatMessage(const QString &, int);
    void notification(const QString &, int);
    void historyRequest(qint64);
    void history(const QStringList &);
};

// This class is instantiated in the client to handle the server channel.
class QCServerChannel : public QCBase
{
    Q_OBJECT
public:
    QCServerChannel();
    bool connectToServer(const QString &, const quint16);
    void sendHistoryRequest();
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
    void sendHistory(const QStringList &, qint64);
private:
    QCChannelServer server_;
    QMap<qint64, QCChannel *> channels_;
    virtual void sendMessage(const QVariantMap &);
private slots:
    void handleChannelConnected(QCChannel *);
    void handleChannelDisconnected();
signals:
    void clientConnected(qint64);
};

#endif // QC_H
