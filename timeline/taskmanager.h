#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "role.h"
#include "task.h"
#include <QObject>
#include <QList>
#include <QSharedPointer>
#include <QHash>
#include <QVariant>

class TaskManager : public QObject
{
    Q_OBJECT

public:   
    static TaskManager &instance();
    void emitUpdated();
    QList<qint64> roleIds() const;
    QList<qint64> taskIds() const;
    QSharedPointer<Role> findRole(qint64) const;
    QSharedPointer<Task> findTask(qint64) const;
    qint64 addRole(const RoleProperties &);
    qint64 addTask(const TaskProperties &);
    void assignTaskToRole(qint64, qint64);
    QList<qint64> assignedTasks(qint64) const;
    void removeRole(qint64);
    void removeTask(qint64);
    void updateRole(qint64, const QHash<QString, QVariant> &);
    void updateTask(qint64, const QHash<QString, QVariant> &);

    void addNewRole(); // ### for now

private:
    TaskManager();

    static bool init_;
    void loadFromSettings();
    void updateSettings();

    QHash<qint64, QSharedPointer<Role> > roles_; // available roles
    QHash<qint64, QSharedPointer<Task> > tasks_; // available tasks
    qint64 nextRoleId_;
    qint64 nextTaskId_;
    int testRoleIndex_; // ### for testing

    void unassignTaskFromRole(qint64, bool = false);

signals:
    void updated();
};

#endif // TASKMANAGER_H
