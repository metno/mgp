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
        QObject::connect(channel, SIGNAL(messageArrived(const QString &)), this, SLOT(handleMessageArrived(const QString &)));
        QObject::connect(channel, SIGNAL(error(const QString &)), this, SLOT(handleError(const QString &)));
        QObject::connect(channel, SIGNAL(disconnected()), this, SLOT(handleDisconnected()));
        channels.append(channel);
    }

private slots:
    void handleMessageArrived(const QString &msg)
    {
        Q_UNUSED(msg);
        // parse msg and act accordingly
    }

    void handleError(const QString &msg)
    {
        Q_UNUSED(msg);
        // report error
    }

    void handleDisconnected()
    {
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
        qFatal(QString("server.init() failed: %1").arg(server.errorString()).toLatin1());
    QCChannelManager cmgr;
    QObject::connect(&server, SIGNAL(channelConnected(QCChannel *)), &cmgr, SLOT(handleChannelConnected(QCChannel *)));

    return app.exec();
}

#include "main.moc"
