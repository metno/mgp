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

qint64 TaskManager::addRole(const RoleProperties &props)
{
    qint64 id = nextRoleId_++;
    roles_.insert(id, QSharedPointer<Role>(new Role(props)));
    return id;
}

qint64 TaskManager::addTask(const TaskProperties &props)
{
    qint64 id = nextTaskId_++;
    tasks_.insert(id, QSharedPointer<Task>(new Task(props)));
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
        tasks_.value(taskId)->roleId_ = -1;
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

void TaskManager::removeRole(qint64 roleId)
{
    if (!roles_.contains(roleId)) return; // no such role
    const QSharedPointer<Role> role = roles_.value(roleId);
    foreach (qint64 taskId, role->taskIds_)
        unassignTaskFromRole(taskId); // for now, we don't remove the tasks themselves
    roles_.remove(roleId);
}

void TaskManager::removeTask(qint64 taskId)
{
    if (!tasks_.contains(taskId)) return; // no such task
    unassignTaskFromRole(taskId);
    tasks_.remove(taskId);
}

void TaskManager::updateRole(qint64 roleId, const QHash<QString, QString> &values)
{
    if (!roles_.contains(roleId)) return; // no such role
    if (values.isEmpty()) return; // no changes

    Role *role = roles_.value(roleId).data();
    role->setName(values.value("name"));
    role->setDescription(values.value("description"));

    emitUpdated();
}

void TaskManager::updateTask(qint64 taskId, const QHash<QString, QString> &values)
{
    if (!tasks_.contains(taskId)) return; // no such task
    if (values.isEmpty()) return; // no changes

    Task *task = tasks_.value(taskId).data();
    task->setName(values.value("name"));
    task->setSummary(values.value("summary"));
    task->setDescription(values.value("description"));

    emitUpdated();
}

void TaskManager::addNewRole()
{
    addRole(RoleProperties(
                QString("test role %1").arg(testRoleIndex_),
                QString("description of test role %1").arg(testRoleIndex_),
                QTime(8, 15), QTime(15, 0)));
    testRoleIndex_++;
    emitUpdated();
}
