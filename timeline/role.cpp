#include "role.h"

Role::Role(const RoleProperties &props)
    : props_(props)
{
}

QString Role::name() const
{
    return props_.name_;
}

void Role::setName(const QString &n)
{
    props_.name_ = n;
}

QString Role::description() const
{
    return props_.description_;
}

void Role::setDescription(const QString &d)
{
    props_.description_ = d;
}

QTime Role::loTime() const
{
    return props_.loTime_;
}

QTime Role::hiTime() const
{
    return props_.hiTime_;
}
