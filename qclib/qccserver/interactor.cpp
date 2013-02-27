// qccserver (using SQLite for database backend)

#include <QtCore> // ### TODO: include relevant headers only
#include <QSqlQuery>
#include <QSqlError>
#include "qcchat.h"
#include "qcglobal.h"
#include "interactor.h"

using namespace qclib;

Interactor::Interactor(QCTcpClientChannels *cchannels, QSqlDatabase *db, int maxagesecs)
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
        cchannels, SIGNAL(windowVisibility(bool, const QString &, const QString &, qint64)),
        SLOT(windowVisibility(bool, const QString &, const QString &, qint64)));
    connect(
        cchannels_, SIGNAL(channelSwitch(int, const QString &, const QString &, qint64)),
        SLOT(channelSwitch(int, const QString &, const QString &, qint64)));
    connect(
        cchannels_, SIGNAL(fullNameChange(const QString &, const QString &, qint64)),
        SLOT(fullNameChange(const QString &, const QString &, qint64)));
    connect(
        cchannels_, SIGNAL(init(const QVariantMap &, qint64)),
        SLOT(init(const QVariantMap &, qint64)));
    connect(cchannels_, SIGNAL(clientDisconnected(qint64)), SLOT(clientDisconnected(qint64)));
}

// Returns a version of \a s with quotes escaped (to reduce possibility of SQL injection etc.).
static QString escapeQuotes(const QString &s)
{
    return QString(s).replace(QRegExp("(')"), "''");
}

void Interactor::db_execTransaction(const QString &query_s)
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
void Interactor::db_deleteOldEvents()
{
    Q_ASSERT(db_);
    Q_ASSERT(db_->isOpen());
    const int oldTimestamp = QDateTime::currentDateTime().toTime_t() - maxagesecs_;
    db_execTransaction(QString("DELETE FROM log WHERE timestamp < %1;").arg(oldTimestamp));
}

void Interactor::db_appendLogEvent(const QString &text, const QString &user, int channelId, int timestamp, int type)
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
QString Interactor::db_setFullName(const QString &user, const QString &fullName)
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

QStringList Interactor::getChannelsFromDatabase()
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

QStringList Interactor::getFullNamesFromDatabase()
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

QStringList Interactor::getHistoryFromDatabase()
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

void Interactor::chatMessage(const QString &text, const QString &user, int channelId)
{
    const int timestamp = QDateTime::currentDateTime().toTime_t();
    cchannels_->sendChatMessage(text, user, channelId, timestamp); // forward to all qclservers
    db_appendLogEvent(text, user, channelId, timestamp, CHATMESSAGE);
}

void Interactor::notification(const QString &text, const QString &user, int channelId)
{
    const int timestamp = QDateTime::currentDateTime().toTime_t();
    cchannels_->sendNotification(text, user, channelId, timestamp); // forward to all qclservers
    db_appendLogEvent(text, user, channelId, timestamp, NOTIFICATION);
}

void Interactor::windowVisibility(bool visible, const QString &, const QString &, qint64 qclserver)
{
    winVisible_.insert(qclserver, visible); // store new value
    cchannels_->sendWindowVisibility(
        visible, user_.value(qclserver), ipaddr_.value(qclserver)); // forward to all qclservers
}

void Interactor::channelSwitch(int channelId, const QString &, const QString &, qint64 qclserver)
{
    channel_.insert(qclserver, channelId); // store new value
    cchannels_->sendChannelSwitch(
        channelId, user_.value(qclserver), ipaddr_.value(qclserver)); // forward to all qclservers
}

void Interactor::fullNameChange(const QString &fullName, const QString &, qint64 qclserver)
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

// Handles init message from a qclserver.
void Interactor::init(const QVariantMap &msg, qint64 qclserver)
{
    Q_ASSERT(!user_.contains(qclserver));
    Q_ASSERT(!ipaddr_.contains(qclserver));
    Q_ASSERT(!winVisible_.contains(qclserver));
    Q_ASSERT(!channel_.contains(qclserver));

    user_.insert(qclserver, msg.value("user").toString());
    const QString ipaddr = msg.value("ipaddr").toString();
    ipaddr_.insert(
        qclserver, ipaddr.isEmpty() ? QString("(IP-address not found; qclserver ID: %1)").arg(qclserver) : ipaddr);

    // send init message to this qclserver only
    QVariantMap msg2;
    // ... general system info
    msg2.insert("hostname", QHostInfo::localHostName());
    msg2.insert("domainname", QHostInfo::localDomainName());
    bool ok;
    const QString ipaddr2 = getLocalIPAddress(&ok);
    if (ok) {
        msg2.insert("ipaddr", ipaddr2);
    } else {
        msg2.insert("ipaddr", "");
        Logger::instance().logError(QString("failed to get local IP address: %1").arg(ipaddr2.toLatin1().data()));
    }
    // ... available chat channels
    msg2.insert("channels", getChannelsFromDatabase());
    // ... full names
    msg2.insert("fullnames", getFullNamesFromDatabase());
    // ... history
    msg2.insert("history", getHistoryFromDatabase());
    //
    cchannels_->sendInit(msg2, qclserver);

    // inform all qclservers about users, their IP-addresses, current window visibilities, and current channels
    QStringList users;
    QStringList ipaddrs;
    QList<bool> winVis;
    QList<int> channels;
    foreach (qint64 qclserver, user_.keys()) {
        users.append(user_.value(qclserver));
        ipaddrs.append(ipaddr_.value(qclserver));
        winVis.append(winVisible_.contains(qclserver) && winVisible_.value(qclserver));
        channels.append(channel_.contains(qclserver) ? channel_.value(qclserver) : -1);
    }
    cchannels_->sendUsers(users, ipaddrs, winVis, channels);
}

void Interactor::clientDisconnected(qint64 qclserver)
{
    user_.remove(qclserver);
    ipaddr_.remove(qclserver);
    winVisible_.remove(qclserver);
    channel_.remove(qclserver);
    cchannels_->sendUsers(user_.values(), ipaddr_.values(), winVisible_.values(), channel_.values());
}
