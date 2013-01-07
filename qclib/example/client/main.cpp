// Example client code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

class QCChannelManager : public QObject
{
    Q_OBJECT
public:
    bool connectToServer(const quint16 port)
    {
        if (!channel.connectToServer(port)) {
            lastError_ = channel.lastError();
            return false;
        }
        connect(&channel, SIGNAL(messageArrived(const QString &)), SLOT(handleMessageArrived(const QString &)));
        connect(&channel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
        connect(&channel, SIGNAL(socketError(QAbstractSocket::SocketError)), SLOT(handleSocketError(QAbstractSocket::SocketError)));
        connect(&channel, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
        return true;
    }

    void sendMessage(const QString &msg)
    {
        channel.sendMessage(msg);
    }

    QString lastError() const
    {
        return lastError_;
    }

private slots:
    void handleMessageArrived(const QString &msg)
    {
        qDebug() << "message arrived:" << msg;
        // parse msg and act accordingly
    }

    void handleChannelError(const QString &msg)
    {
        qDebug() << "channel error:" << msg;
        lastError_ = msg;
    }

    void handleSocketError(QAbstractSocket::SocketError e)
    {
        handleChannelError(QString("socket error: %1").arg(e));
    }

    void handleChannelDisconnected()
    {
        qDebug() << "channel disconnected (2 B IMPLEMENTED)";
        // remove sender() from channels
    }

private:
    QCChannel channel;
    QString lastError_;
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCChannelManager cmgr;
    const int port = 1104;
    if (!cmgr.connectToServer(port))
        qFatal("cmgr.connectToServer() failed: %s", cmgr.lastError().toLatin1().data());
    cmgr.sendMessage("open_chat_win");
    cmgr.sendMessage("close_chat_win");
    cmgr.sendMessage("send_irc_msg ...");

    return app.exec();
}

#include "main.moc"
