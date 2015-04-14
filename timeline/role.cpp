#include "role.h"

Role::Role(const QString &name__, const QTime &beginTime__, const QTime &endTime__)
    : name_(name__)
    , beginTime_(beginTime__)
    , endTime_(endTime__)
{
}

QString Role::name() const
{
    return name_;
}

QTime Role::beginTime() const
{
    return beginTime_;
}

QTime Role::endTime() const
{
    return endTime_;
}
