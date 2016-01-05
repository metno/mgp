#include "taskmanager.h"
#include "common.h"
#include <QSharedPointer>
#include <QSettings>

extern QSharedPointer<QSettings> settings;

TaskManager::TaskManager()
    : nextRoleId_(0)
    , nextTaskId_(0)
    , testRoleIndex_(0)
{
}

bool TaskManager::init_ = false;

TaskManager &TaskManager::instance()
{
    static TaskManager tm;
    if (!init_) {
        tm.loadFromSettings();
        init_ = true;
    }
    return tm;
}

void TaskManager::emitUpdated()
{
    emit updated();
}

void TaskManager::loadFromSettings()
{
    if (!settings) {
        qWarning() << "WARNING: task manager: no valid settings file found; tasks and roles not loaded";
        return;
    }

    Q_ASSERT(tasks_.empty());
    Q_ASSERT(roles_.empty());

    // load tasks
    const int ntasks = settings->beginReadArray("tasks");
    for (int i = 0; i < ntasks; ++i) {
        settings->setArrayIndex(i);
        Task *task = new Task;
        task->setRoleId(settings->value("roleId").toLongLong());
        task->setName(settings->value("name").toString());
        task->setSummary(settings->value("summary").toString());
        task->setDescription(settings->value("description").toString());
        task->setLoDateTime(settings->value("loDateTime").toDateTime());
        task->setHiDateTime(settings->value("hiDateTime").toDateTime());

        const qint64 id = settings->value("id").toLongLong();
        tasks_.insert(id, QSharedPointer<Task>(task));
        nextTaskId_ = qMax(nextTaskId_, id + 1);
    }
    settings->endArray();

    // load roles
    const int nroles = settings->beginReadArray("roles");
    for (int i = 0; i < nroles; ++i) {
        settings->setArrayIndex(i);
        Role *role = new Role;
        role->setName(settings->value("name").toString());
        role->setDescription(settings->value("description").toString());
        role->setLoTime(settings->value("loTime").toTime());
        role->setHiTime(settings->value("hiTime").toTime());

        const qint64 id = settings->value("id").toLongLong();
        roles_.insert(id, QSharedPointer<Role>(role));
        nextRoleId_ = qMax(nextRoleId_, id + 1);
    }
    settings->endArray();

    // assign tasks to roles:
    foreach (qint64 taskId, tasks_.keys()) {
        if (tasks_.value(taskId)->roleId() >= 0)
            assignTaskToRole(taskId, tasks_.value(taskId)->roleId());
    }
}

void TaskManager::updateSettings()
{
    int index = -1;

    // save tasks
    settings->beginWriteArray("tasks");
    index = 0;
    foreach (quint64 id, tasks_.keys()) {
        settings->setArrayIndex(index++);
        settings->setValue("id", id);
        const QSharedPointer<Task> task = tasks_.value(id);
        settings->setValue("roleId", task->roleId());
        settings->setValue("name", task->name());
        settings->setValue("summary", task->summary());
        settings->setValue("description", task->description());
        settings->setValue("loDateTime", task->loDateTime());
        settings->setValue("hiDateTime", task->hiDateTime());
    }
    settings->endArray();

    // save roles
    settings->beginWriteArray("roles");
    index = 0;
    foreach (quint64 id, roles_.keys()) {
        settings->setArrayIndex(index++);
        settings->setValue("id", id);
        const QSharedPointer<Role> role = roles_.value(id);
        settings->setValue("name", role->name());
        settings->setValue("description", role->description());
        settings->setValue("loTime", role->loTime());
        settings->setValue("hiTime", role->hiTime());
    }
    settings->endArray();

    settings->sync();
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
    updateSettings();
    return id;
}

qint64 TaskManager::addTask(const TaskProperties &props)
{
    qint64 id = nextTaskId_++;
    tasks_.insert(id, QSharedPointer<Task>(new Task(props)));
    updateSettings();
    return id;
}

void TaskManager::unassignTaskFromRole(qint64 taskId, bool skipUpdateSettings)
{
    Q_ASSERT(tasks_.contains(taskId));

    // unassign from original role if any
    const qint64 origRoleId = tasks_.value(taskId)->roleId_;
    if (origRoleId >= 0) {
        const QSharedPointer<Role> origRole = roles_.value(origRoleId);
        Q_ASSERT(origRole);
        origRole->taskIds_.removeOne(taskId);
        tasks_.value(taskId)->roleId_ = -1;
        if (!skipUpdateSettings)
            updateSettings();
    }
}

void TaskManager::assignTaskToRole(qint64 taskId, qint64 roleId)
{
    if (!tasks_.contains(taskId)) return; // no such task
    if (!roles_.contains(roleId)) return; // no such role

    unassignTaskFromRole(taskId, true);

    // assign to new role
    tasks_.value(taskId)->roleId_ = roleId;
    const QSharedPointer<Role> role = roles_.value(roleId);
    Q_ASSERT(!role->taskIds_.contains(taskId));
    role->taskIds_.append(taskId);
    updateSettings();
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
        unassignTaskFromRole(taskId, true); // for now, we don't remove the tasks themselves
    roles_.remove(roleId);
    updateSettings();
}

void TaskManager::removeTask(qint64 taskId)
{
    if (!tasks_.contains(taskId)) return; // no such task
    unassignTaskFromRole(taskId, true);
    tasks_.remove(taskId);
    updateSettings();
}

void TaskManager::updateRole(qint64 roleId, const QHash<QString, QVariant> &values)
{
    if (!roles_.contains(roleId)) return; // no such role
    if (values.isEmpty()) return; // no changes

    Role *role = roles_.value(roleId).data();
    role->setName(values.value("name").toString());
    role->setLoTime(values.value("loTime").toTime());
    role->setHiTime(values.value("hiTime").toTime());
    role->setDescription(values.value("description").toString());

    emitUpdated();
    updateSettings();
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
    updateSettings();
}

void TaskManager::addNewRole()
{
    addRole(RoleProperties(
                QString("test role %1").arg(testRoleIndex_),
                QString("description of test role %1").arg(testRoleIndex_),
                QTime(8, 15), QTime(15, 0)));
    testRoleIndex_++;
    emitUpdated();
    updateSettings();
}
