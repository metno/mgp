// Example server code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

class QCChannelManager : public QObject
{
    Q_OBJECT
public:
    void broadcast(const QString &msg)
    {
        foreach (QCChannel *channel, channels)
            channel->sendMessage(msg);
    }

public slots:
    void handleChannelConnected(QCChannel *channel)
    {
        qDebug() << "new client connected:" << channel->peerInfo().toLatin1().data();
        connect(channel, SIGNAL(messageArrived(const QString &)), SLOT(handleMessageArrived(const QString &)));
        connect(channel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
        connect(channel, SIGNAL(socketDisconnected()), SLOT(handleChannelDisconnected()));
        channels.append(channel);
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
    }

    void handleChannelDisconnected()
    {
        QCChannel *channel = static_cast<QCChannel *>(sender());
        qDebug() << "client disconnected:" << channel->peerInfo().toLatin1().data();
        channels.removeOne(channel);
        channel->deleteLater(); // ### or 'delete channel' directly?
    }

private:
    QList<QCChannel *> channels;
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCChannelServer server;
    const int port = 1104;
    if (!server.listen(port))
        qFatal("server.init() failed: %s", server.lastError().toLatin1().data());
    QCChannelManager cmgr;
    QObject::connect(&server, SIGNAL(channelConnected(QCChannel *)), &cmgr, SLOT(handleChannelConnected(QCChannel *)));

    return app.exec();
}

#include "main.moc"
