#include "task.h"

Task::Task(const TaskProperties &props)
    : roleId_(-1)
    , props_(props)
{
}

qint64 Task::roleId() const
{
    return roleId_;
}

void Task::setRoleId(qint64 id)
{
    roleId_ = id;
}

QString Task::name() const
{
    return props_.name_;
}

void Task::setName(const QString &n)
{
    props_.name_ = n;
}

QString Task::summary() const
{
    return props_.summary_;
}

void Task::setSummary(const QString &s)
{
    props_.summary_ = s;
}

QString Task::description() const
{
    return props_.description_;
}

void Task::setDescription(const QString &d)
{
    props_.description_ = d;
}

QDateTime Task::loDateTime() const
{
    return props_.loDateTime_;
}

void Task::setLoDateTime(const QDateTime &dt)
{
    props_.loDateTime_ = dt;
}

void Task::setLoUTCTimestamp(int t)
{
    props_.loDateTime_.setTime_t(t);
}

QDateTime Task::hiDateTime() const
{
    return props_.hiDateTime_;
}

void Task::setHiDateTime(const QDateTime &dt)
{
    props_.hiDateTime_ = dt;
}

void Task::setHiUTCTimestamp(int t)
{
    props_.hiDateTime_.setTime_t(t);
}
