#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "role.h"
#include "task.h"
#include <QObject>
#include <QList>
#include <QSharedPointer>
#include <QHash>

class TaskManager : public QObject
{
    Q_OBJECT
public:   
    TaskManager();
    static TaskManager *instance();
    void emitUpdated();
    QList<qint64> roleIds() const;
    QList<qint64> taskIds() const;
    QSharedPointer<Role> findRole(qint64) const;
    QSharedPointer<Task> findTask(qint64) const;
    qint64 addRole(const QSharedPointer<Role> &);
    qint64 addTask(const QSharedPointer<Task> &);
    void assignTaskToRole(qint64, qint64);
    QList<qint64> assignedTasks(qint64) const;

private:
    QHash<qint64, QSharedPointer<Role> > roles_; // available roles
    QHash<qint64, QSharedPointer<Task> > tasks_; // available tasks
    static TaskManager *self_;   // singleton instance pointer
    qint64 nextRoleId_;
    qint64 nextTaskId_;

signals:
    void updated();
};

#endif // TASKMANAGER_H
