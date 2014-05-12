#ifndef ITEM_H
#define ITEM_H

#include <QList>
#include <QVariantMap>
#include <QDomElement>
#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
#include <QPointF>
#include <iostream>

struct Item {
  QList<QPointF> points; // lat,lon points
  QVariantMap properties;
  QDomElement createExtDataElement(QDomDocument &) const;
  QDomElement createPointOrPolygonElement(QDomDocument &) const;
  QDomElement createTimeSpanElement(QDomDocument &) const;
  QDomElement createPlacemarkElement(QDomDocument &) const;
  QDomNode toKML() const;
};

#endif // ITEM_H
