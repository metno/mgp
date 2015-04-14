#include "laneitem.h"
#include <QBrush>
#include <QColor>

LaneItem::LaneItem(qint64 roleId__)
    : roleId_(roleId__)
{
    setBrush(QBrush(QColor("#eee")));
}
