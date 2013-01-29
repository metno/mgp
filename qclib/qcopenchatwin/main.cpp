// qcopenchatwin

#include <QtCore> // ### TODO: include relevant headers only
#include "qcchat.h"

class Interactor : public QObject
{
    Q_OBJECT
#define TIMEOUT 3000 // timeout in milliseconds
public:
    Interactor(QCServerChannel *schannel)
    {
        // exit as soon as the server confirms that the window is shown or TIMEOUT milliseconds have passed
        connect(schannel, SIGNAL(showChatWindow()), SLOT(serverResponse()));
        QTimer::singleShot(TIMEOUT, this, SLOT(noServerResponse()));
    }
private slots:
    void serverResponse()
    {
        qDebug() << "response from server; terminating normally";
        qApp->exit(0);
    }
    void noServerResponse()
    {
        qDebug("WARNING: no response from server within %d milliseconds; terminating prematurely", TIMEOUT);
        qApp->exit(0);
    }
};

static void printUsage()
{
    qDebug() << QString("usage: %1 --lport <local server port>").arg(qApp->arguments().first()).toLatin1().data();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // extract command-line options
    const QMap<QString, QString> options = getOptions(app.arguments());
    bool ok;
    const quint16 lport = options.value("lport").toUInt(&ok);
    if (!ok) {
        qDebug() << "failed to extract local server port";
        printUsage();
        return 1;
    }

    // establish channel to qclserver
    QCServerChannel schannel;
    if (!schannel.connectToServer("localhost", lport)) {
        qDebug("schannel.connectToServer() failed: %s", schannel.lastError().toLatin1().data());
        return 1;
    }

    // create object to handle interaction with qclserver
    Interactor interactor(&schannel);

    // request chat window to be shown
    schannel.sendShowChatWindow();

    return app.exec();
}

#include "main.moc"
