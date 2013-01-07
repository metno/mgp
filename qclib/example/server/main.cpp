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
        connect(channel, SIGNAL(messageArrived(const QString &)), SLOT(handleMessageArrived(const QString &)));
        connect(channel, SIGNAL(error(const QString &)), SLOT(handleChannelError(const QString &)));
        connect(channel, SIGNAL(socketError(QAbstractSocket::SocketError)), SLOT(handleSocketError(QAbstractSocket::SocketError)));
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
        qDebug() << "channel error:" << msg;
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
