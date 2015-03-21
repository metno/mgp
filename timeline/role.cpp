#include "role.h"

Role::Role(const QString &name__)
    : name_(name__)
{
}

QString Role::name() const
{
    return name_;
}

void Role::setName(const QString &name__)
{
    name_ = name__;
}
