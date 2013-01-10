// Example qccserver code.

#include <QtCore> // ### TODO: include relevant headers only
#include "qc.h"

class Interactor : public QObject
{
    Q_OBJECT
public:
    Interactor(QCClientChannels *cchannels /*, Database *db ...*/)
        : cchannels(cchannels)
    {
        connect(cchannels, SIGNAL(chatMessage(const QString &)), SLOT(chatMessage(const QString &)));
        connect(cchannels, SIGNAL(notification(const QString &)), SLOT(notification(const QString &)));
    }

private:
    QCClientChannels *cchannels; // qclserver channels

private slots:
    void chatMessage(const QString &msg)
    {
        qDebug() << "chat message (from a qclserver):" << msg;
        cchannels->sendChatMessage(msg); // forward to all qclservers
        // append to database ... 2 B DONE!
    }

    void notification(const QString &msg)
    {
        qDebug() << "notification (from a qclserver):" << msg;
        cchannels->sendNotification(msg); // forward to all qclservers
        // append to database ... 2 B DONE!
    }
};


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // extract information from environment
    bool ok;
    const quint16 qccport = qgetenv("QCCPORT").toUInt(&ok);
    if (!ok) {
        qDebug("failed to extract int from environment variable QCCPORT");
        return 1;
    }
    // extract from QCDBFILE ... 2 B DONE!

    // listen for incoming qclserver connections
    QCClientChannels cchannels;
    if (!cchannels.listen(qccport)) {
        qDebug(
            "failed to listen for incoming qclserver connections: cchannels.listen() failed: %s",
            cchannels.lastError().toLatin1().data());
        return 1;
    }

    // create a database (based on SQLite)
    //Database *db = ...

    // create object to handle interaction between qclservers and the database
    Interactor interactor(&cchannels/*, db*/);

    return app.exec();
}

#include "main.moc"
