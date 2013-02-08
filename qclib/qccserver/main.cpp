// qccserver (using SQLite for database backend)

#include <QtCore> // ### TODO: include relevant headers only
#include <QSqlQuery>
#include <QSqlError>
#include "qcchat.h"
#include "qcglobal.h"

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

    // log
    ok = query.exec(
        "CREATE TABLE log(id INTEGER PRIMARY KEY AUTOINCREMENT"
        ", text TEXT NOT NULL"
        ", user TEXT NOT NULL"
        ", channelId INTEGER REFERENCES channel(id) NOT NULL"
        ", timestamp INTEGER NOT NULL"
        ", type INTEGER NOT NULL);");
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
        qWarning("WARNING: no match for IP address: %s", line.toLatin1().data());
        return QString();
    }
    return rx.cap(1);
}


class Interactor : public QObject
{
    Q_OBJECT
public:
    Interactor(QCTcpClientChannels *cchannels, QSqlDatabase *db)
        : cchannels_(cchannels)
        , db_(db)
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
            cchannels_, SIGNAL(init(const QVariantMap &, qint64)),
            SLOT(init(const QVariantMap &, qint64)));
        connect(cchannels_, SIGNAL(clientDisconnected(qint64)), SLOT(clientDisconnected(qint64)));
    }

private:
    QCTcpClientChannels *cchannels_; // qclserver channels
    QSqlDatabase *db_;
    QMap<qint64, QString> user_;
    QMap<qint64, int> channel_; // current chat channel (a.k.a. chat room)

    // Returns a version of \a s with quotes escaped (to reduce possibility of SQL injection etc.).
    static QString escapeQuotes(const QString &s)
    {
        return QString(s).replace(QRegExp("(')"), "''");
    }

    void appendToDatabase(const QString &text, const QString &user, int channelId, int timestamp, int type)
    {
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());
        QScopedPointer<QSqlQuery> query(new QSqlQuery(*db_));
        QString query_s = QString(
            "INSERT INTO log (text, user, channelId, timestamp, type) VALUES ('%1', '%2', %3, %4, %5);")
            .arg(escapeQuotes(text)).arg(escapeQuotes(user)).arg(channelId).arg(timestamp).arg(type);
        db_->transaction();
        if (!query->exec(query_s))
            qWarning(
                "WARNING: query '%s' failed: %s",
                query_s.toLatin1().data(), query->lastError().text().trimmed().toLatin1().data());
        db_->commit();
    }

    QStringList getChannelsFromDatabase()
    {
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());
        QScopedPointer<QSqlQuery> query(new QSqlQuery(*db_));
        QString query_s = QString("SELECT id, name, description FROM channel;");
        QStringList c;
        if (!query->exec(query_s)) {
            qWarning(
                "WARNING: query '%s' failed: %s",
                query_s.toLatin1().data(), query->lastError().text().trimmed().toLatin1().data());
        } else while (query->next()) {
             const int id = query->value(0).toInt();
             const QString name = query->value(1).toString();
             const QString descr = query->value(2).toString();
             c << QString("%1 %2 %3").arg(id).arg(name).arg(descr);
        }
        return c;
    }

    QStringList getHistoryFromDatabase()
    {
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());
        QScopedPointer<QSqlQuery> query(new QSqlQuery(*db_));
        QString query_s = QString("SELECT text, user, channelId, timestamp, type FROM log ORDER BY timestamp;");
        QStringList h;
        if (!query->exec(query_s)) {
            qWarning(
                "WARNING: query '%s' failed: %s",
                query_s.toLatin1().data(), query->lastError().text().trimmed().toLatin1().data());
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
        appendToDatabase(text, user, channelId, timestamp, CHATMESSAGE);
    }

    void notification(const QString &text, const QString &user, int channelId)
    {
        const int timestamp = QDateTime::currentDateTime().toTime_t();
        cchannels_->sendNotification(text, user, channelId, timestamp); // forward to all qclservers
        appendToDatabase(text, user, channelId, timestamp, NOTIFICATION);
    }

    void channelSwitch(int channelId, const QString &, qint64 qclserver)
    {
        channel_.insert(qclserver, channelId); // store new value
        cchannels_->sendChannelSwitch(channelId, user_.value(qclserver)); // forward to all qclservers
    }

    void init(const QVariantMap &msg, qint64 qclserver)
    {
        const QString user(msg.value("user").toString());

        if (user_.values().contains(user)) {
            qWarning("WARNING: user '%s' already joined; disconnecting", user.toLatin1().data());
            cchannels_->close(qclserver);
            return;
        }

        user_.insert(qclserver, user);
        Q_ASSERT(!channel_.contains(qclserver));

        // send init message to this qccserver only
        QVariantMap msg2;
        // ... general system info
        msg2.insert("hostname", QHostInfo::localHostName());
        msg2.insert("domainname", QHostInfo::localDomainName());
        msg2.insert("ipaddr", getLocalIPAddress());
        // ... available chat channels
        msg2.insert("channels", getChannelsFromDatabase());
        // ... history
        msg2.insert("history", getHistoryFromDatabase());
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

static void printUsage()
{
    qDebug() << QString(
        "usage: %1 --dbfile <SQLite dtaabase file> (--initdb | (--cport <central server port>))")
        .arg(qApp->arguments().first()).toLatin1().data();
}

int main(int argc, char *argv[])
{
    // the following enables support for unicode chars (like 'æ') in
    // SQL INSERT statements etc.
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QCoreApplication app(argc, argv);

    // extract information from environment
    const QMap<QString, QString> options = getOptions(app.arguments());
    bool ok;
    const QString dbfile = options.value("dbfile");
    if (dbfile.isEmpty()) {
        qDebug("failed to extract database file name");
        printUsage();
        return 1;
    }
    if (options.contains("initdb")) {
        // initialize database and terminate
        if (QFile::exists(dbfile)) {
            qDebug() << QString("database file already exists: %1; please (re)move it first")
                .arg(dbfile).toLatin1().data();
            return 1;
        }
        QString error;
        if (!initDatabase(dbfile, &error)) {
            qDebug() << "failed to initialize new database:" << error.toLatin1().data();
            return 1;
        }
        return 0;
    } else if (!QFile::exists(dbfile)) {
        qDebug() << "database file not found:" << dbfile.toLatin1().data();
        return 1;
    }
    const quint16 cport = options.value("cport").toUInt(&ok);
    if (!ok) {
        qDebug("failed to extract central server port");
        printUsage();
        return 1;
    }

    // listen for incoming qclserver connections, and allow connections associated with any user
    QCTcpClientChannels cchannels;
    if (!cchannels.listen(cport)) {
        qDebug(
            "failed to listen for incoming qclserver connections: cchannels.listen() failed: %s",
            cchannels.lastError().toLatin1().data());
        return 1;
    }

    // open database
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbfile);
    if (!db.open()) {
        qDebug() << "failed to open database:" << dbfile;
        return 1;
    }

    // create object to handle interaction between qclservers and the database
    Interactor interactor(&cchannels, &db);

    return app.exec();
}

#include "main.moc"
