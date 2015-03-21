#include "task.h"

Task::Task(const QString &name__)
    : name_(name__)
    , roleId_(-1)
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
