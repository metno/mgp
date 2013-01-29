#ifndef QCCHAT_H
#define QCCHAT_H

#include "qc.h"

// Chat event types
#define CHATMESSAGE 0
#define NOTIFICATION 1

class QCBase : public QObject
{
    Q_OBJECT
public:
    QString lastError() const;
protected:
    QCBase();
    enum MsgType {
        ShowChatWin, HideChatWin, ChatMsg, Notification, Initialization, Channels, History, Users, ChannelSwitch
    };
    void setLastError(const QString &);
private:
    QString lastError_;
    virtual void sendMessage(const QVariantMap &) = 0;
public slots:
    void sendShowChatWindow(qint64 = -1);
    void sendHideChatWindow(qint64 = -1);
    void sendChatMessage(const QString &, const QString &, int, int = -1);
    void sendNotification(const QString &, const QString & = QString(), int = -1, int = -1);
    void sendChannelSwitch(int, const QString & = QString());
protected slots:
    void handleMessageArrived(qint64, const QVariantMap &);
    void handleChannelError(const QString &);
signals:
    void showChatWindow();
    void hideChatWindow();
    void chatMessage(const QString &, const QString &, int, int);
    void notification(const QString &, const QString &, int, int);
    void initialization(qint64, const QVariantMap &msg);
    void channels(const QStringList &);
    void history(const QStringList &);
    void users(const QStringList &, const QList<int> &);
    void channelSwitch(qint64, int, const QString &);
};

// This class is instantiated in the client to handle the server channel.
class QCServerChannel : public QCBase
{
    Q_OBJECT
public:
    QCServerChannel();
    bool connectToServer(const QString &, const quint16);
    bool isConnected() const;
    void initialize(const QVariantMap &msg);
private:
    QCChannel *channel_;
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
    virtual ~QCClientChannels();
    bool listen(const qint16);
    void close(qint64);
    void sendChannels(const QStringList &, qint64);
    void sendHistory(const QStringList &, qint64);
    void sendUsers(const QStringList &, const QList<int> &);
private:
    QCChannelServer server_;
    QMap<qint64, QCChannel *> channels_;
    virtual void sendMessage(const QVariantMap &);
private slots:
    void handleChannelConnected(QCChannel *);
    void handleChannelDisconnected();
signals:
    void clientConnected(qint64);
    void clientDisconnected(qint64);
};

#endif // QCCHAT_H
