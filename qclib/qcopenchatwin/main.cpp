// qcopenchatwin

#include <QtGui> // ### TODO: include relevant headers only
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
            Logger::instance().logError(
                QString("central host mismatch: %1 != %2")
                .arg(msg.value("chost").toString().toLatin1().data())
                .arg(chost_.toLatin1().data()));
            qApp->exit(1);
            return;
        }
        bool ok;
        const quint16 cport = msg.value("cport").toInt(&ok);
        if (!ok) {
            Logger::instance().logError("central port not an integer");
            qApp->exit(1);
            return;
        }
        if (cport != cport_) {
            Logger::instance().logError(QString("central port mismatch: %1 != %2").arg(cport).arg(cport_));
            qApp->exit(1);
            return;
        }

        // chost and cport both match so request chat window to be shown if necessary
        if (!msg.value("windowshown").toBool()) {
            connect(schannel_, SIGNAL(showChatWindow()), SLOT(showChatWindow()));
            schannel_->sendShowChatWindow();
        } else {
            //Logger::instance().logInfo("window already shown; terminating normally");
            qApp->exit(0);
        }
    }

    void showChatWindow()
    {
        //Logger::instance().logInfo("window shown by our request; terminating normally");
        qApp->exit(0);
    }

    void serverInitTimeout()
    {
        Logger::instance().logError(
            QString("no response from server within %1 milliseconds; terminating prematurely").arg(TIMEOUT));
        qApp->exit(1);
    }
};

static void printUsage()
{
    Logger::instance().logError(
        QString("usage: %1 --chost <central server host> --cport <central server port>")
        .arg(qApp->arguments().first().toLatin1().data()));
}

int main(int argc, char *argv[])
{
    Logger::instance().initialize("/tmp/qcopenchatwin.log");

    QApplication app(argc, argv);

    // extract command-line options
    const QMap<QString, QString> options = getOptions(app.arguments());
    const QString chost = options.value("chost");
    if (chost.isEmpty()) {
        Logger::instance().logError("failed to extract central server host");
        printUsage();
        return 1;
    }
    bool ok;
    const quint16 cport = options.value("cport").toUInt(&ok);
    if (!ok) {
        Logger::instance().logError("failed to extract central server port");
        printUsage();
        return 1;
    }

    // establish channel to qclserver
    QCLocalServerChannel schannel;
    if (!schannel.connectToServer(chost, cport)) {
        Logger::instance().logError(
            QString("schannel.connectToServer() failed: %1. "
                "Central server at %2:%3 is probably down.")
            .arg(schannel.lastError().toLatin1().data())
            .arg(chost).arg(cport));
        QMessageBox::critical(
            0, "Metno Chat Client ERROR",
            QString("Failed to open chat window.\nCentral server at %1:%2 is probably down.").arg(chost).arg(cport));
        return 1;
    }

    // create object to handle interaction
    Interactor interactor(&schannel, chost, cport);

    return app.exec();
}

#include "main.moc"
