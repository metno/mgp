// qccserver (using SQLite for database backend)

#include <QtCore> // ### TODO: include relevant headers only
#include <QSqlQuery>
#include <QSqlError>
#include "qcchat.h"
#include "qcglobal.h"
#include <sys/types.h>
#include <sys/stat.h>

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


static QString getLocalIPAddress()
{
    QProcess p1;
    QProcess p2;
    p1.setStandardOutputProcess(&p2);
    p1.start("ifconfig eth0");
    p2.start("grep", QStringList() << "inet addr");
    p1.waitForFinished(-1);
    p2.waitForFinished(-1);
    const QString line = p2.readAllStandardOutput();
    QRegExp rx("inet addr:(\\d+\\.\\d+\\.\\d+\\.\\d+)");
    if (rx.indexIn(line) == -1) {
        Logger::instance().logWarning(QString("no match for IP address: %1").arg(line.toLatin1().data()));
        return QString();
    }
    return rx.cap(1);
}


class Interactor : public QObject
{
    Q_OBJECT
public:
    Interactor(QCTcpClientChannels *cchannels, QSqlDatabase *db, int maxagesecs)
        : cchannels_(cchannels)
        , db_(db)
        , maxagesecs_(maxagesecs)
    {
        connect(
            cchannels_, SIGNAL(chatMessage(const QString &, const QString &, int, int)),
            SLOT(chatMessage(const QString &, const QString &, int)));
        connect(
            cchannels_, SIGNAL(notification(const QString &, const QString &, int, int)),
            SLOT(notification(const QString &, const QString &, int)));
        connect(
            cchannels_, SIGNAL(channelSwitch(int, const QString &, qint64)),
            SLOT(channelSwitch(int, const QString &, qint64)));
        connect(
            cchannels_, SIGNAL(fullNameChange(const QString &, const QString &, qint64)),
            SLOT(fullNameChange(const QString &, const QString &, qint64)));
        connect(
            cchannels_, SIGNAL(init(const QVariantMap &, qint64)),
            SLOT(init(const QVariantMap &, qint64)));
        connect(cchannels_, SIGNAL(clientDisconnected(qint64)), SLOT(clientDisconnected(qint64)));
    }

private:
    QCTcpClientChannels *cchannels_; // qclserver channels
    QSqlDatabase *db_;
    QMap<qint64, QString> user_;
    QMap<qint64, int> channel_; // current chat channel (a.k.a. chat room)
    int maxagesecs_; // max age (in secs) for database events (older events are removed)

    // Returns a version of \a s with quotes escaped (to reduce possibility of SQL injection etc.).
    static QString escapeQuotes(const QString &s)
    {
        return QString(s).replace(QRegExp("(')"), "''");
    }

    void db_execTransaction(const QString &query_s)
    {
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());
        QScopedPointer<QSqlQuery> query(new QSqlQuery(*db_));
        db_->transaction();
        if (!query->exec(query_s))
            Logger::instance().logWarning(
                QString("query '%1' failed: %2")
                .arg(query_s.toLatin1().data())
                .arg(query->lastError().text().trimmed().toLatin1().data()));
        db_->commit();
    }

    // Deletes events (chat messages and notifications) older than a certain age from the database.
    // This limits the growth of the database.
    void db_deleteOldEvents()
    {
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());
        const int oldTimestamp = QDateTime::currentDateTime().toTime_t() - maxagesecs_;
        db_execTransaction(QString("DELETE FROM log WHERE timestamp < %1;").arg(oldTimestamp));
    }

    void db_appendLogEvent(const QString &text, const QString &user, int channelId, int timestamp, int type)
    {
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());
        db_execTransaction(
            QString(
            "INSERT INTO log (text, user, channelId, timestamp, type) VALUES ('%1', '%2', %3, %4, %5);")
            .arg(escapeQuotes(text)).arg(escapeQuotes(user)).arg(channelId).arg(timestamp).arg(type));
        db_deleteOldEvents();
    }

    // Updates the full name of a user if not already taken by another user. Returns the user associated with the
    // full name after the operation, or an empty string if an error occurred.
    QString db_setFullName(const QString &user, const QString &fullName)
    {
        Q_ASSERT(!user.isEmpty());
        //Q_ASSERT(!fullName.isEmpty());
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());

        // check if the non-empty full name already exists for a different user
        if (!fullName.trimmed().isEmpty()) {
            QScopedPointer<QSqlQuery> query(new QSqlQuery(*db_));
            QString query_s =
                QString("SELECT user FROM fullname WHERE user != '%1' AND trim(lower(fullname)) = '%2';")
                .arg(user).arg(fullName.toLower().trimmed());
            if (!query->exec(query_s)) {
                Logger::instance().logWarning(
                    QString("query '%s' failed: %s")
                    .arg(query_s.toLatin1().data())
                    .arg(query->lastError().text().trimmed().toLatin1().data()));
                return QString();
            } else if (query->next()) {
                // full name in use for a different user
                return query->value(0).toString();
            }
        }

        // update the full name for this user
        db_execTransaction(
            QString(
            "INSERT OR REPLACE INTO fullname (user, fullname) VALUES ('%1', '%2');")
            .arg(escapeQuotes(user)).arg(escapeQuotes(fullName.trimmed())));

        return user;
    }

    QStringList getChannelsFromDatabase()
    {
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());
        QScopedPointer<QSqlQuery> query(new QSqlQuery(*db_));
        QString query_s = QString("SELECT id, name, description FROM channel;");
        QStringList c;
        if (!query->exec(query_s)) {
            Logger::instance().logWarning(
                QString("query '%s' failed: %s")
                .arg(query_s.toLatin1().data())
                .arg(query->lastError().text().trimmed().toLatin1().data()));
        } else while (query->next()) {
                const int id = query->value(0).toInt();
                const QString name = query->value(1).toString();
                const QString descr = query->value(2).toString();
                c << QString("%1 %2 %3").arg(id).arg(name).arg(descr);
            }
        return c;
    }

    QStringList getFullNamesFromDatabase()
    {
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());
        QScopedPointer<QSqlQuery> query(new QSqlQuery(*db_));
        QString query_s = QString("SELECT user, fullname FROM fullname;");
        QStringList f;
        if (!query->exec(query_s)) {
            Logger::instance().logWarning(
                QString("query '%s' failed: %s")
                .arg(query_s.toLatin1().data())
                .arg(query->lastError().text().trimmed().toLatin1().data()));
        } else while (query->next()) {
             const QString user = query->value(0).toString();
             const QString fullName = query->value(1).toString();
             f << QString("%1 %2").arg(user).arg(fullName);
        }
        return f;
    }

    QStringList getHistoryFromDatabase()
    {
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());
        QScopedPointer<QSqlQuery> query(new QSqlQuery(*db_));
        QString query_s = QString("SELECT text, user, channelId, timestamp, type FROM log ORDER BY timestamp;");
        QStringList h;
        if (!query->exec(query_s)) {
            Logger::instance().logWarning(
                QString("query '%s' failed: %s")
                .arg(query_s.toLatin1().data())
                .arg(query->lastError().text().trimmed().toLatin1().data()));
        } else while (query->next()) {
             const QString text = query->value(0).toString();
             const QString user = query->value(1).toString();
             const int channelId = query->value(2).toInt();
             const int timestamp = query->value(3).toInt();
             const int type = query->value(4).toInt();
             h << QString("%1 %2 %3 %4 %5").arg(user).arg(channelId).arg(timestamp).arg(type).arg(text);
        }
        return h;
    }

private slots:
    void chatMessage(const QString &text, const QString &user, int channelId)
    {
        const int timestamp = QDateTime::currentDateTime().toTime_t();
        cchannels_->sendChatMessage(text, user, channelId, timestamp); // forward to all qclservers
        db_appendLogEvent(text, user, channelId, timestamp, CHATMESSAGE);
    }

    void notification(const QString &text, const QString &user, int channelId)
    {
        const int timestamp = QDateTime::currentDateTime().toTime_t();
        cchannels_->sendNotification(text, user, channelId, timestamp); // forward to all qclservers
        db_appendLogEvent(text, user, channelId, timestamp, NOTIFICATION);
    }

    void channelSwitch(int channelId, const QString &, qint64 qclserver)
    {
        channel_.insert(qclserver, channelId); // store new value
        cchannels_->sendChannelSwitch(channelId, user_.value(qclserver)); // forward to all qclservers
    }

    void fullNameChange(const QString &fullName, const QString &, qint64 qclserver)
    {
        const QString user = user_.value(qclserver);
        const QString currUser = db_setFullName(user, fullName);
        if (currUser.isEmpty())
            return; // an error occurred
        if (currUser == user)
            cchannels_->sendFullNameChange(fullName, user); // forward successful update to all qclservers
        else
            // report conflict to this qclserver only
            cchannels_->sendErrorMessage(
                QString("Full name '%1' already taken by user '%2'.").arg(fullName).arg(currUser), qclserver);
    }

    void init(const QVariantMap &msg, qint64 qclserver)
    {
        const QString user(msg.value("user").toString());

        if (user_.values().contains(user)) {
            Logger::instance().logWarning(QString("user '%1' already joined; disconnecting").arg(user.toLatin1().data()));
            cchannels_->close(qclserver);
            return;
        }

        user_.insert(qclserver, user);
        Q_ASSERT(!channel_.contains(qclserver));

        // send init message to this qclserver only
        QVariantMap msg2;
        // ... general system info
        msg2.insert("hostname", QHostInfo::localHostName());
        msg2.insert("domainname", QHostInfo::localDomainName());
        msg2.insert("ipaddr", getLocalIPAddress());
        // ... available chat channels
        msg2.insert("channels", getChannelsFromDatabase());
        // ... full names
        msg2.insert("fullnames", getFullNamesFromDatabase());
        // ... history
        msg2.insert("history", getHistoryFromDatabase());
        //
        cchannels_->sendInit(msg2, qclserver);

        // inform all qclservers about users and their current channels
        QStringList users;
        QList<int> channels;
        foreach (qint64 qclserver, user_.keys()) {
            users.append(user_.value(qclserver));
            channels.append(channel_.contains(qclserver) ? channel_.value(qclserver) : -1);
        }
        cchannels_->sendUsers(users, channels);
    }

    void clientDisconnected(qint64 qclserver)
    {
        const QString user = user_.take(qclserver);
        channel_.remove(qclserver);
        cchannels_->sendUsers(user_.values(), channel_.values());
    }
};

static void printUsage(const QString &appName = QString(), bool toLogger = true)
{
    const QString s = QString(
        "usage: %1 [--daemon] --dbfile <SQLite dtaabase file> (--initdb | (--cport <central server port> "
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

#include "main.moc"
