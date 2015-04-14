#ifndef ROLE_H
#define ROLE_H

#include <QString>
#include <QList>
#include <QTime>

class Role
{
    friend class TaskManager;

public:
    Role(const QString &, const QTime &, const QTime &);
    QString name() const;
    QTime beginTime() const;
    QTime endTime() const;

private:
    QString name_;
    QList<qint64> taskIds_; // tasks assigned to this role

    // time interval in which the role is active
    QTime beginTime_;
    QTime endTime_;
};

#endif // ROLE_H
