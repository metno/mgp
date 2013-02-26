// qccserver (using SQLite for database backend)

#include <QtCore> // ### TODO: include relevant headers only
#include <QSqlQuery>
#include <QSqlError>
#include "qcchat.h"
#include "qcglobal.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "interactor.h"

using namespace qclib;

static bool initDatabase(const QString &dbfile, QString *error)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbfile);
    if (!db.open()) {
        *error = QString("failed to open database: \"%1\"").arg(dbfile);
        return false;
    }

    QSqlQuery query;

    bool ok;

    // *** Create tables ***

    // channels (a.k.a. 'chat rooms')
    ok = query.exec(
        "CREATE TABLE channel(id INTEGER PRIMARY KEY AUTOINCREMENT"
        ", name TEXT NOT NULL"
        ", description TEXT NOT NULL"
        ", UNIQUE(name)"
        ", UNIQUE(description));");
    Q_ASSERT(ok);
    ok = query.exec("INSERT INTO channel(name, description) VALUES ('profet', '<beskrivelse av profet-kanalen>');");
    Q_ASSERT(ok);
    ok = query.exec("INSERT INTO channel(name, description) VALUES ('analyse', '<beskrivelse av analyse-kanalen>');");
    Q_ASSERT(ok);
    ok = query.exec("INSERT INTO channel(name, description) VALUES ('flyvær', '<beskrivelse av flyvær-kanalen>');");
    Q_ASSERT(ok);

    // log (i.e. timestamped chat events such as normal chat messages, notifications etc.)
    ok = query.exec(
        "CREATE TABLE log(id INTEGER PRIMARY KEY AUTOINCREMENT"
        ", text TEXT NOT NULL"
        ", user TEXT NOT NULL"
        ", channelId INTEGER REFERENCES channel(id) NOT NULL"
        ", timestamp INTEGER NOT NULL"
        ", type INTEGER NOT NULL);");
    Q_ASSERT(ok);

    // fullname (i.e. the full (real) names of users)
    ok = query.exec(
        "CREATE TABLE fullname(id INTEGER PRIMARY KEY AUTOINCREMENT"
        ", user TEXT NOT NULL"
        ", fullname TEXT NOT NULL"
        ", UNIQUE(user)"
        ", UNIQUE(fullname));");
    Q_ASSERT(ok);

    // *** Create indexes ***
    // ...

    return true;
}

static void printUsage(const QString &appName = QString(), bool toLogger = true)
{
    const QString s = QString(
        "usage: %1 [--daemon] --dbfile <SQLite database file> (--initdb | (--cport <central server port> "
        "--maxage <max # of days to keep chat events>))")
        .arg((!appName.isEmpty() ? appName : qApp->applicationName()).toLatin1().data());
    if (toLogger)
        Logger::instance().logError(s);
    else
        qDebug() << s.toLatin1().data();
}

// Turns process into a daemon.
static void daemonize()
{
    pid_t pid, sid; // our process ID and session ID

    pid = fork(); // fork off parent process
    if (pid < 0)
        exit(1); // fork() failed
    if (pid > 0)
        exit(0); // exit parent

    // at this point we're the child

    umask(0); // provide full access to generated files (in particular log files)

    sid = setsid(); // create new SID for the child
    if (sid < 0) {
        Logger::instance().logError("setsid() failed");
        exit(1);
    }

    // change current working directory
    if ((chdir("/")) < 0) {
        Logger::instance().logError("chdir(\"/\") failed");
        exit(1);
    }

    // close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main(int argc, char *argv[])
{
    Logger::instance().initialize("/tmp/qccserver.log");

    const QMap<QString, QString> options = getOptions(argc, argv);
    if (options.contains("help")) {
        printUsage(argv[0], false);
        return 0;
    }

    // check if we should run as a daemon
    if (options.contains("daemon"))
        daemonize();

    // the following enables support for unicode chars (like 'æ') in
    // SQL INSERT statements etc.
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QCoreApplication app(argc, argv);

    // extract other input arguments
    bool ok;
    const QString dbfile = options.value("dbfile");
    if (dbfile.isEmpty()) {
        Logger::instance().logError("failed to extract database file name");
        printUsage();
        return 1;
    }
    if (options.contains("initdb")) {
        // initialize database and terminate
        if (QFile::exists(dbfile)) {
            Logger::instance().logError(
                QString("database file already exists: %1; please (re)move it first")
                .arg(dbfile).toLatin1().data());
            return 1;
        }
        QString error;
        if (!initDatabase(dbfile, &error)) {
            Logger::instance().logError(QString("failed to initialize new database: %1").arg(error.toLatin1().data()));
            return 1;
        }
        return 0;
    } else if (!QFile::exists(dbfile)) {
        Logger::instance().logError(QString("database file not found: %1").arg(dbfile.toLatin1().data()));
        return 1;
    }
    const quint16 cport = options.value("cport").toUInt(&ok);
    if (!ok) {
        Logger::instance().logError("failed to extract central server port");
        printUsage();
        return 1;
    }
    const int maxagedays = options.value("maxage").toInt(&ok);
    if (!ok) {
        Logger::instance().logError("failed to extract maxage as integer");
        printUsage();
        return 1;
    }
    if (maxagedays < 0) {
        Logger::instance().logError("maxage cannot be a negative value");
        printUsage();
        return 1;
    }

    // listen for incoming qclserver connections, and allow connections associated with any user
    QCTcpClientChannels cchannels;
    if (!cchannels.listen(cport)) {
        Logger::instance().logError(
            QString("failed to listen for incoming qclserver connections: cchannels.listen() failed: %1")
            .arg(cchannels.lastError().toLatin1().data()));
        return 1;
    }

    // open database
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbfile);
    if (!db.open()) {
        Logger::instance().logError(QString("failed to open database: %1").arg(dbfile));
        return 1;
    }

    // create object to handle interaction between qclservers and the database
    Interactor interactor(&cchannels, &db, maxagedays * 24 * 60 * 60);

    Logger::instance().logInfo(
        QString("server started at %1").arg(QDateTime::currentDateTime().toString("yyyy MMM dd hh:mm:ss")));
    return app.exec();
}
