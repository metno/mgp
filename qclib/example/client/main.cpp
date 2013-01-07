// Example client code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

class QCChannelManager : public QObject
{
    Q_OBJECT
public:
    bool connect(const quint16 port)
    {
        if (!channel.connect(port))
            return false;
        QObject::connect(&channel, SIGNAL(messageArrived(const QString &)), this, SLOT(handleMessageArrived(const QString &)));
        QObject::connect(&channel, SIGNAL(error(const QString &)), this, SLOT(handleError(const QString &)));
        QObject::connect(&channel, SIGNAL(disconnected()), this, SLOT(handleDisconnected()));
        return true;
    }

    void sendMessage(const QString &msg)
    {
        channel.sendMessage(msg);
    }

    QString errorString() const
    {
        return "unimplemented";
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
    QCChannel channel;
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCChannelManager cmgr;
    const int port = 1104;
    if (!cmgr.connect(port))
        qFatal(QString("cmgr.connect() failed: %1").arg(cmgr.errorString()).toLatin1());
    cmgr.sendMessage("open_chat_win");
    cmgr.sendMessage("close_chat_win");
    cmgr.sendMessage("send_irc_msg ...");

    return app.exec();
}

#include "main.moc"
