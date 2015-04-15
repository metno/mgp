#include "taskmanager.h"

TaskManager *TaskManager::self_ = 0;

TaskManager::TaskManager()
    : nextRoleId_(0)
    , nextTaskId_(0)
    , testRoleIndex_(0)
{
}

TaskManager *TaskManager::instance()
{
  if (!TaskManager::self_)
    TaskManager::self_ = new TaskManager();
  return TaskManager::self_;
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

void TaskManager::assignTaskToRole(qint64 taskId, qint64 roleId)
{
    if (!tasks_.contains(taskId)) return; // no such task
    if (!roles_.contains(roleId)) return; // no such role

    const QSharedPointer<Task> task = tasks_.value(taskId);
    const QSharedPointer<Role> role = roles_.value(roleId);

    // unassign from original role if any
    const qint64 origRoleId = task->roleId_;
    if (origRoleId >= 0) {
        const QSharedPointer<Role> origRole = roles_.value(origRoleId);
        Q_ASSERT(origRole);
        origRole->taskIds_.removeOne(taskId);
    }

    // assign to new role
    task->roleId_ = roleId;
    Q_ASSERT(!role->taskIds_.contains(taskId));
    role->taskIds_.append(taskId);
}

QList<qint64> TaskManager::assignedTasks(qint64 roleId) const
{
    if (!roles_.contains(roleId)) return QList<qint64>(); // no such role

    return roles_.value(roleId)->taskIds_;
}

void TaskManager::add5Roles()
{
    for (int i = 0; i < 5; ++i)
        addRole(QSharedPointer<Role>(new Role(QString("test role %1").arg(testRoleIndex_++), QTime(3 + i, 0), QTime(11, 0))));

    emitUpdated();
}
