// qcopenchatwin

#include <QtCore> // ### TODO: include relevant headers only
#include "qcchat.h"
#include "qcglobal.h"

using namespace qclib;

class Interactor : public QObject
{
    Q_OBJECT
#define TIMEOUT 3000 // server init timeout in milliseconds
public:
    Interactor(QCLocalServerChannel *schannel, const QString &chost, quint16 cport)
        : schannel_(schannel)
        , chost_(chost)
        , cport_(cport)
    {
        // await init message or timeout
        connect(schannel, SIGNAL(init(const QVariantMap &, qint64)), SLOT(init(const QVariantMap &)));
        QTimer::singleShot(TIMEOUT, this, SLOT(serverInitTimeout()));
    }
private:
    QCLocalServerChannel *schannel_;
    QString chost_;
    quint16 cport_;

private slots:
    void init(const QVariantMap &msg)
    {
        if (msg.value("chost").toString() != chost_) {
            qDebug(
                "ERROR: central host mismatch: %s != %s", msg.value("chost").toString().toLatin1().data(),
                chost_.toLatin1().data());
            qApp->exit(1);
            return;
        }
        bool ok;
        const quint16 cport = msg.value("cport").toInt(&ok);
        if (!ok) {
            qDebug("ERROR: central port not an integer");
            qApp->exit(1);
            return;
        }
        if (cport != cport_) {
            qDebug("ERROR: central port mismatch: %d != %d",  cport, cport_);
            qApp->exit(1);
            return;
        }

        // chost and cport both match so request chat window to be shown if necessary
        if (!msg.value("windowshown").toBool()) {
            connect(schannel_, SIGNAL(showChatWindow()), SLOT(showChatWindow()));
            schannel_->sendShowChatWindow();
        } else {
            qDebug("window already shown; terminating normally");
            qApp->exit(0);
        }
    }

    void showChatWindow()
    {
        qDebug("window shown by our request; terminating normally");
        qApp->exit(0);
    }

    void serverInitTimeout()
    {
        qDebug("ERROR: no response from server within %d milliseconds; terminating prematurely", TIMEOUT);
        qApp->exit(1);
    }
};

static void printUsage()
{
    qDebug(
        "usage: %s --chost <central server host> --cport <central server port>",
        qApp->arguments().first().toLatin1().data());
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // extract command-line options
    const QMap<QString, QString> options = getOptions(app.arguments());
    const QString chost = options.value("chost");
    if (chost.isEmpty()) {
        qDebug("failed to extract central server host");
        printUsage();
        return 1;
    }
    bool ok;
    const quint16 cport = options.value("cport").toUInt(&ok);
    if (!ok) {
        qDebug("failed to extract central server port");
        printUsage();
        return 1;
    }

    // establish channel to qclserver
    QCLocalServerChannel schannel;
    if (!schannel.connectToServer(chost, cport)) {
        qDebug("schannel.connectToServer() failed: %s", schannel.lastError().toLatin1().data());
        return 1;
    }

    // create object to handle interaction
    Interactor interactor(&schannel, chost, cport);

    return app.exec();
}

#include "main.moc"
