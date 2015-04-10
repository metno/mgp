#include "leftheaderbgitem.h"
#include <QBrush>
#include <QColor>

LeftHeaderBGItem::LeftHeaderBGItem(qint64 roleId__)
    : roleId_(roleId__)
{
    setBrush(QBrush(QColor(192 + qrand() % 64, 192 + qrand() % 64, 192 + qrand() % 64)));
}
