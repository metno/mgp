#ifndef QCCHAT_H
#define QCCHAT_H

#include "qc.h"

namespace qclib {

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
        Init, WindowVisibility, ChatMsg, Notification, Channels, History, Users, ChannelSwitch, FullNameChange,
        ErrorMessage
    };
    void setLastError(const QString &);
private:
    QString lastError_;
    virtual void sendMessage(const QVariantMap &) = 0;
public slots:
    void sendChatMessage(const QString &, const QString &, int, int = -1);
    void sendNotification(const QString &, const QString & = QString(), int = -1, int = -1);
    void sendWindowVisibility(bool, const QString & = QString(), const QString & = QString());
    void sendShowWindow() { sendWindowVisibility(true); }
    void sendHideWindow() { sendWindowVisibility(false); }
    void sendChannelSwitch(int, const QString & = QString(), const QString & = QString());
    void sendFullNameChange(const QString &, const QString & = QString());
protected slots:
    void handleMessageArrived(qint64, const QVariantMap &);
    void handleChannelError(const QString &);
signals:
    void init(const QVariantMap &, qint64);
    void chatMessage(const QString &, const QString &, int, int);
    void notification(const QString &, const QString &, int, int);
    void channels(const QStringList &);
    void history(const QStringList &);
    void users(const QStringList &, const QStringList &, const QList<bool> &, const QList<int> &);
    void windowVisibility(bool, const QString &, const QString &, qint64);
    void channelSwitch(int, const QString &, const QString &, qint64);
    void fullNameChange(const QString &, const QString &, qint64);
    void errorMessage(const QString &, qint64);
};

class QCServerChannel : public QCBase
{
    Q_OBJECT
public:
    bool isConnected() const;
    void sendInit(const QVariantMap &);
protected:
    QCServerChannel(QCChannel * = 0);
    QCChannel *channel_;
    virtual void sendMessage(const QVariantMap &);
private slots:
    void handleChannelDisconnected();
signals:
    void serverDisconnected();
};

// This class is instantiated in a qcapp to handle communication with the qclserver.
class QCLocalServerChannel : public QCServerChannel
{
    Q_OBJECT
public:
    QCLocalServerChannel();
    bool connectToServer(const QString &, quint16);
};

// This class is instantiated in a qclserver to handle communication with the qccserver.
class QCTcpServerChannel : public QCServerChannel
{
    Q_OBJECT
public:
    QCTcpServerChannel();
    bool connectToServer(const QString &, const quint16);
};

class QCClientChannels : public QCBase
{
    Q_OBJECT
public:
    QCClientChannels();
    virtual ~QCClientChannels();
    void sendInit(const QVariantMap &, qint64);
    void sendUsers(const QStringList &, const QStringList &, const QList<bool> &, const QList<int> &);
    void sendErrorMessage(const QString &, qint64);
    void close(qint64);
protected:
    QMap<qint64, QCChannel *> channels_;
    virtual void sendMessage(const QVariantMap &);
private slots:
    void handleChannelConnected(QCChannel *);
    void handleChannelDisconnected();
signals:
    void clientConnected(qint64);
    void clientDisconnected(qint64);
};

// This class is instantiated in the qclserver to handle communication with qcapps.
class QCLocalClientChannels : public QCClientChannels
{
    Q_OBJECT
public:
    QCLocalClientChannels();
    bool listen();
private:
    QCLocalChannelServer server_;
signals:
    void serverFileChanged(const QString &);
};

// This class is instantiated in the qccserver to handle communication with qclservers.
class QCTcpClientChannels : public QCClientChannels
{
    Q_OBJECT
public:
    QCTcpClientChannels();
    bool listen(const qint16);
private:
    QCTcpChannelServer server_;
};

} // namespace qclib

#endif // QCCHAT_H
