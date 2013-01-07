// Example client code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

class QCChannelManager : public QObject
{
    Q_OBJECT
public:
    QCChannelManager()
        : channel(0)
    {
    }

    bool connectToServer(const quint16 port)
    {
        if (channel) {
            lastError_ = "channel already connected, please disconnect first";
            return false;
        }

        channel = new QCChannel;

        if (!channel->connectToServer(port)) {
            lastError_ = channel->lastError();
            return false;
        }
        connect(channel, SIGNAL(messageArrived(const QString &)), SLOT(handleMessageArrived(const QString &)));
        connect(channel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
        connect(channel, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
        return true;
    }

    void sendMessage(const QString &msg)
    {
        if (!channel) {
            const char *emsg = "channel not connected";
            qWarning("%s", emsg);
            lastError_ = emsg;
            return;
        }
        channel->sendMessage(msg);
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
        qDebug() << "channel error:" << msg.toLatin1().data();
        lastError_ = msg;
    }

    void handleChannelDisconnected()
    {
        Q_ASSERT(channel);
        qDebug() << "server disconnected:" << channel->peerInfo().toLatin1().data();
        channel->deleteLater();
        channel = 0;
    }

private:
    QCChannel *channel;
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
