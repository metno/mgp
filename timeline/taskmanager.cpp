#include "taskmanager.h"

TaskManager::TaskManager()
    : nextRoleId_(0)
    , nextTaskId_(0)
    , testRoleIndex_(0)
{
}

TaskManager &TaskManager::instance()
{
    static TaskManager tm;
    return tm;
}

void TaskManager::emitUpdated()
{
    emit updated();
}

QList<qint64> TaskManager::roleIds() const
{
    return roles_.keys();
}

QList<qint64> TaskManager::taskIds() const
{
    return tasks_.keys();
}

QSharedPointer<Role> TaskManager::findRole(qint64 id) const
{
    return roles_.value(id);
}

QSharedPointer<Task> TaskManager::findTask(qint64 id) const
{
    return tasks_.value(id);
}

qint64 TaskManager::addRole(const QSharedPointer<Role> &role)
{
    qint64 id = nextRoleId_++;
    roles_.insert(id, role);
    return id;
}

qint64 TaskManager::addTask(const QSharedPointer<Task> &task)
{
    qint64 id = nextTaskId_++;
    tasks_.insert(id, task);
    return id;
}

void TaskManager::unassignTaskFromRole(qint64 taskId)
{
    Q_ASSERT(tasks_.contains(taskId));

    // unassign from original role if any
    const qint64 origRoleId = tasks_.value(taskId)->roleId_;
    if (origRoleId >= 0) {
        const QSharedPointer<Role> origRole = roles_.value(origRoleId);
        Q_ASSERT(origRole);
        origRole->taskIds_.removeOne(taskId);
    }
}

void TaskManager::assignTaskToRole(qint64 taskId, qint64 roleId)
{
    if (!tasks_.contains(taskId)) return; // no such task
    if (!roles_.contains(roleId)) return; // no such role

    unassignTaskFromRole(taskId);

    // assign to new role
    tasks_.value(taskId)->roleId_ = roleId;
    const QSharedPointer<Role> role = roles_.value(roleId);
    Q_ASSERT(!role->taskIds_.contains(taskId));
    role->taskIds_.append(taskId);
}

QList<qint64> TaskManager::assignedTasks(qint64 roleId) const
{
    if (!roles_.contains(roleId)) return QList<qint64>(); // no such role

    return roles_.value(roleId)->taskIds_;
}

void TaskManager::removeTask(qint64 taskId)
{
    if (!tasks_.contains(taskId)) return; // no such task
    unassignTaskFromRole(taskId);
    tasks_.remove(taskId);
}

void TaskManager::add5Roles()
{
    for (int i = 0; i < 5; ++i)
        addRole(QSharedPointer<Role>(new Role(QString("test role %1").arg(testRoleIndex_++), QTime(3 + i, 0), QTime(11, 0))));

    emitUpdated();
}
