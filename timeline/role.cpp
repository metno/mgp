#include "role.h"

Role::Role(const RoleProperties &props)
    : props_(props)
{
}

QString Role::name() const
{
    return props_.name_;
}

QString Role::description() const
{
    return props_.description_;
}

QTime Role::loTime() const
{
    return props_.loTime_;
}

QTime Role::hiTime() const
{
    return props_.hiTime_;
}
