#ifndef ROLE_H
#define ROLE_H

#include <QString>
#include <QList>

class Role
{
    friend class TaskManager;
public:
    Role(const QString &);
    QString name() const;
    void setName(const QString &);
private:
    QString name_;
    QList<qint64> taskIds_; // tasks assigned to this role
};

#endif // ROLE_H
