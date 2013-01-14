// Example qccserver code (based on SQLite).

#include <QtCore> // ### TODO: include relevant headers only
#include <QSqlQuery>
#include <QSqlError>
#include "qc.h"

#define CHATMESSAGE 0
#define NOTIFICATION 1

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
        ", timestamp INTEGER NOT NULL"
        ", type INTEGER NOT NULL"
        ", value TEXT NOT NULL);");
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
        : cchannels(cchannels)
        , db(db)
    {
        connect(cchannels, SIGNAL(chatMessage(const QString &)), SLOT(chatMessage(const QString &)));
        connect(cchannels, SIGNAL(notification(const QString &)), SLOT(notification(const QString &)));
        connect(cchannels, SIGNAL(historyRequest(qint64)), SLOT(historyRequest(qint64)));
    }

private:
    QCClientChannels *cchannels; // qclserver channels
    QSqlDatabase *db;

    void appendToDatabase(int type, const QString &msg)
    {
        Q_ASSERT(db);
        Q_ASSERT(db->isOpen());
        uint timestamp = QDateTime::currentDateTime().toTime_t();
        QSqlQuery *query = new QSqlQuery(*db);
        db->transaction();
        QString query_s = QString(
            "INSERT INTO log (timestamp, type, value) VALUES (%1, %2, '%3');").arg(timestamp).arg(type).arg(msg);
        if (!query->exec(query_s)) {
            qDebug() << "query failed:" << query->lastError().text().trimmed();
            // hm ... maybe this is only required for a SELECT:
            // if (query->isActive())
            //     query->finish();
        }

        db->commit();
    }

private slots:
    void chatMessage(const QString &msg)
    {
        qDebug() << "chat message (from a qclserver):" << msg;
        cchannels->sendChatMessage(msg); // forward to all qclservers
        appendToDatabase(CHATMESSAGE, msg);
    }

    void notification(const QString &msg)
    {
        qDebug() << "notification (from a qclserver):" << msg;
        cchannels->sendNotification(msg); // forward to all qclservers
        appendToDatabase(NOTIFICATION, msg);
    }

    void historyRequest(qint64 qclserver)
    {
        qDebug() << QString("history request (from qclserver %1)").arg(qclserver).toLatin1().data();
        // for now (eventually extract from DB):
        QStringList h = QStringList() << "chat msg 1" << "chat msg 2" << "notification 1" << "chat msg 3";
        cchannels->sendHistory(h, qclserver);
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
