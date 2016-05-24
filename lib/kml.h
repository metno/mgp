#ifndef KML_H
#define KML_H

#include "mgp.h"
#include <QString>
#include <QByteArray>

#define KML_BEGIN_NAMESPACE namespace kml {
#define KML_END_NAMESPACE }

KML_BEGIN_NAMESPACE

mgp::Polygons kml2polygons(const QByteArray &, QString *);

KML_END_NAMESPACE

#endif // KML_H
