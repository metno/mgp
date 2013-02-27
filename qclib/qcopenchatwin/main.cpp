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

        // chost and cport both match so request chat window to become visible if necessary
        if (!msg.value("windowvisible").toBool()) {
            connect(
                schannel_, SIGNAL(windowVisibility(bool, const QString &, const QString &, qint64)),
                SLOT(windowVisibility(bool)));
            schannel_->sendShowWindow();
        } else {
            //Logger::instance().logInfo("window already visible; terminating normally");
            qApp->exit(0);
        }
    }

    void windowVisibility(bool visible)
    {
        if (!visible) {
            // Logger::instance().logError("ERROR: expected visible == true");
        } else {
            // Logger::instance().logInfo("window shown by our request; terminating normally");
        }
        qApp->exit(0);
    }

    void serverInitTimeout()
    {
        Logger::instance().logError(
            QString("no response from server within %1 milliseconds; terminating prematurely").arg(TIMEOUT));
        qApp->exit(1);
    }
};

static void printUsage(bool toLogger = true)
{
    const QString s = QString(
        "usage: %1 --help | ([--chost <central server host> (default: metchat.met.no)] "
        "[--cport <central server port> (default: 1105)])")
        .arg(qApp->arguments().first().toLatin1().data());
    if (toLogger)
        Logger::instance().logError(s);
    else
        qDebug() << s.toLatin1().data();
}

int main(int argc, char *argv[])
{
    Logger::instance().initialize("/tmp/qcopenchatwin.log");

    QApplication app(argc, argv);

    // extract command-line options
    const QMap<QString, QString> options = getOptions(app.arguments());
    if (options.contains("help")) {
        printUsage(false);
        return 0;
    }
    QString chost = options.value("chost");
    if (chost.isEmpty())
        chost = "metchat.met.no";
    const QString cport_s = options.value("cport");
    quint16 cport;
    if (cport_s.isEmpty())
        cport = 1105;
    else {
        bool ok;
        cport = cport_s.toUInt(&ok);
        if (!ok) {
            Logger::instance().logError(
                QString("failed to extract central server port as unsigned integer: %1").arg(cport_s));
            return 1;
        }
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
            0, "MetChat ERROR",
            QString("Failed to open chat window.\nCentral server at %1:%2 is probably down.").arg(chost).arg(cport));
        return 1;
    }

    // create object to handle interaction
    Interactor interactor(&schannel, chost, cport);

    return app.exec();
}

#include "main.moc"
