#include "task.h"

Task::Task(const QString &name__, const QDateTime &loDateTime__, const QDateTime &hiDateTime__)
    : name_(name__)
    , roleId_(-1)
    , loDateTime_(loDateTime__)
    , hiDateTime_(hiDateTime__)
{
}

QString Task::name() const
{
    return name_;
}

void Task::setName(const QString &name__)
{
    name_ = name__;
}

qint64 Task::roleId() const
{
    return roleId_;
}

QDateTime Task::loDateTime() const
{
    return loDateTime_;
}

QDateTime Task::hiDateTime() const
{
    return hiDateTime_;
}
