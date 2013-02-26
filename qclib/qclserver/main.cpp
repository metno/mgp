// qclserver

#include <QtGui> // ### TODO: include relevant headers only
#include "qcglobal.h"
#include "interactor.h"
#include <csignal>

using namespace qclib;

QSettings *settings = 0;

static void printUsage(bool toLogger = true)
{
    const QString s = QString(
        "usage: %1 --help | (--chost <central server host> --cport <central server port>)")
        .arg(qApp->arguments().first().toLatin1().data());
    if (toLogger)
        Logger::instance().logError(s);
    else
        qDebug() << s.toLatin1().data();
}

struct CleanExit {
    CleanExit() {
        signal(SIGQUIT, &CleanExit::exitQt);
        signal(SIGINT, &CleanExit::exitQt);
        signal(SIGTERM, &CleanExit::exitQt);
        // signal(SIGBREAK, &CleanExit::exitQt);
        // ### TODO: Add more signals to cover all relevant reasons for termination
        // ...
    }

    static void exitQt(int sig) {
        Q_UNUSED(sig);
        QCoreApplication::exit(0);
    }
};

class ExitHandler : public QObject
{
    Q_OBJECT
public slots:
    void cleanup()
    {
        // do cleanup actions here
    }
};

int main(int argc, char *argv[])
{
    Logger::instance().initialize("/tmp/qclserver.log");
    CleanExit cleanExit;
    QApplication app(argc, argv);
    ExitHandler exitHandler;
    QObject::connect(&app, SIGNAL(aboutToQuit()), &exitHandler, SLOT(cleanup()));

    // extract command-line options
    const QMap<QString, QString> options = getOptions(app.arguments());
    if (options.contains("help")) {
        printUsage(false);
        return 0;
    }
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

    // create global settings object for this $USER (assuming 1-1 correspondence between $USER and $HOME)
    settings = new QSettings(
        QString("%1/.config/metchat/qclserver.conf").arg(qgetenv("HOME").constData()), QSettings::NativeFormat);

    // create object to handle interaction between qcapps, qccserver, and chat window
    Interactor interactor(qgetenv("USER").constData(), chost, cport);
    if (!interactor.initialize()) {
        Logger::instance().logError("failed to initialize interactor");
        return 1;
    }

    return app.exec();
}

#include "main.moc"
