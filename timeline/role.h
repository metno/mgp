#ifndef ROLE_H
#define ROLE_H

#include <QString>
#include <QList>
#include <QTime>

class RoleProperties
{
    friend class Role;

public:
    RoleProperties(const QString &name, const QString &description, const QTime &loTime, const QTime &hiTime)
        : name_(name)
        , description_(description)
        , loTime_(loTime)
        , hiTime_(hiTime)
    {
    }

private:
    QString name_;
    QString description_;
    QTime loTime_;
    QTime hiTime_;
};

class Role
{
    friend class TaskManager;

public:
    QString name() const;
    QString description() const;
    QTime loTime() const;
    QTime hiTime() const;

private:
    Role(const RoleProperties &);

    QList<qint64> taskIds_; // tasks assigned to this role
    RoleProperties props_;

    void setName(const QString &);
    void setLoTime(const QTime &);
    void setHiTime(const QTime &);
    void setDescription(const QString &);
};

#endif // ROLE_H
