#ifndef INTERACTOR_H
#define INTERACTOR_H

#include <QtCore> // ### TODO: include relevant headers only
#include "qcchat.h"

using namespace qclib;

class QSqlDatabase;

class Interactor : public QObject
{
    Q_OBJECT
public:
    Interactor(QCTcpClientChannels *, QSqlDatabase *db, int);
private:
    QCTcpClientChannels *cchannels_; // qclserver channels
    QSqlDatabase *db_;
    QMap<qint64, QString> user_; // user associated with a qclserver
    QMap<qint64, QString> ipaddr_; // IP-address associated with a qclserver
    QMap<qint64, bool> winVisible_; // current chat window visibility associated with a qclserver
    QMap<qint64, int> channel_; // current chat channel (a.k.a. chat room) associated with a qclserver
    int maxagesecs_; // max age (in secs) for database events (older events are removed)

    void db_execTransaction(const QString &);
    void db_deleteOldEvents();
    void db_appendLogEvent(const QString &, const QString &, int, int, int);
    QString db_setFullName(const QString &, const QString &);
    QStringList getChannelsFromDatabase();
    QStringList getFullNamesFromDatabase();
    QStringList getHistoryFromDatabase();
private slots:
    void chatMessage(const QString &, const QString &, int);
    void notification(const QString &, const QString &, int);
    void windowVisibility(bool, const QString &, const QString &, qint64);
    void channelSwitch(int, const QString &, const QString &, qint64);
    void fullNameChange(const QString &, const QString &, qint64);
    void init(const QVariantMap &, qint64);
    void clientDisconnected(qint64);
};

#endif // INTERACTOR_H
