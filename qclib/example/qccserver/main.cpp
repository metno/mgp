// Example qccserver code (based on SQLite).

#include <QtCore> // ### TODO: include relevant headers only
#include <QSqlQuery>
#include <QSqlError>
#include "qc.h"

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

    // log
    ok = query.exec(
        "CREATE TABLE log(id INTEGER PRIMARY KEY AUTOINCREMENT"
        ", text TEXT NOT NULL"
        ", user TEXT NOT NULL"
        ", timestamp INTEGER NOT NULL"
        ", type INTEGER NOT NULL);");
    Q_ASSERT(ok);

    // *** Create indexes ***
    // ...

    return true;
}

class Interactor : public QObject
{
    Q_OBJECT
public:
    Interactor(QCClientChannels *cchannels, QSqlDatabase *db)
        : cchannels_(cchannels)
        , db_(db)
    {
        connect(
            cchannels_, SIGNAL(chatMessage(const QString &, const QString &, int)),
            SLOT(chatMessage(const QString &, const QString &)));
        connect(
            cchannels_, SIGNAL(notification(const QString &, const QString &, int)),
            SLOT(notification(const QString &, const QString &)));
        connect(cchannels_, SIGNAL(historyRequest(qint64)), SLOT(historyRequest(qint64)));
    }

private:
    QCClientChannels *cchannels_; // qclserver channels
    QSqlDatabase *db_;

    // Returns a version of \a s with quotes escaped (to reduce possibility of SQL injection etc.).
    static QString escapeQuotes(const QString &s)
    {
        return QString(s).replace(QRegExp("(')"), "''");
    }

    void appendToDatabase(const QString &text, const QString &user, int timestamp, int type)
    {
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());
        QScopedPointer<QSqlQuery> query(new QSqlQuery(*db_));
        QString query_s = QString(
            "INSERT INTO log (text, user, timestamp, type) VALUES ('%1', '%2', %3, %4);")
            .arg(escapeQuotes(text)).arg(escapeQuotes(user)).arg(timestamp).arg(type);
        db_->transaction();
        if (!query->exec(query_s))
            qWarning(
                "WARNING: query '%s' failed: %s",
                query_s.toLatin1().data(), query->lastError().text().trimmed().toLatin1().data());
        db_->commit();
    }

    QStringList getHistoryFromDatabase()
    {
        Q_ASSERT(db_);
        Q_ASSERT(db_->isOpen());
        QScopedPointer<QSqlQuery> query(new QSqlQuery(*db_));
        QString query_s = QString("SELECT text, user, timestamp, type FROM log ORDER BY timestamp;");
        QStringList h;
        if (!query->exec(query_s)) {
            qWarning(
                "WARNING: query '%s' failed: %s",
                query_s.toLatin1().data(), query->lastError().text().trimmed().toLatin1().data());
        } else while (query->next()) {
             const QString text = query->value(0).toString();
             const QString user = query->value(1).toString();
             const int timestamp = query->value(2).toInt();
             const int type = query->value(3).toInt();
             h << QString("%1 %2 %3 %4").arg(user).arg(timestamp).arg(type).arg(text);
        }
        return h;
    }

private slots:
    void chatMessage(const QString &text, const QString &user)
    {
        qDebug() << QString("chat message (from a qclserver; user: %1): %2").arg(user).arg(text).toLatin1().data();
        const int timestamp = QDateTime::currentDateTime().toTime_t();
        cchannels_->sendChatMessage(text, user, timestamp); // forward to all qclservers
        appendToDatabase(text, user, timestamp, CHATMESSAGE);
    }

    void notification(const QString &text, const QString &user)
    {
        qDebug() << QString("notification (from a qclserver; user: %1): %2").arg(user).arg(text).toLatin1().data();
        const int timestamp = QDateTime::currentDateTime().toTime_t();
        cchannels_->sendNotification(text, user, timestamp); // forward to all qclservers
        appendToDatabase(text, user, timestamp, NOTIFICATION);
    }

    void historyRequest(qint64 qclserver)
    {
        qDebug() << QString("history request (from qclserver channel %1)").arg(qclserver).toLatin1().data();
        QStringList h = getHistoryFromDatabase();
        cchannels_->sendHistory(h, qclserver);
    }
};


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // extract information from environment
    bool ok;
    const QString dbfile = qgetenv("QCDBFILE");
    if (dbfile.isEmpty()) {
        qDebug("failed to extract string from environment variable QCDBFILE");
        return 1;
    }
    const QString initdb = qgetenv("QCINITDB");
    if (!initdb.isEmpty() && initdb != "0" && initdb.toLower() != "false" && initdb.toLower() != "no") {
        // initialize DB and terminate
        if (QFile::exists(dbfile)) {
            qDebug() << "database file already exists:" << dbfile.toLatin1().data();
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
    const quint16 qccport = qgetenv("QCCPORT").toUInt(&ok);
    if (!ok) {
        qDebug("failed to extract int from environment variable QCCPORT");
        return 1;
    }

    // listen for incoming qclserver connections
    QCClientChannels cchannels;
    if (!cchannels.listen(qccport)) {
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
