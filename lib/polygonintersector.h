#ifndef POLYGONINTERSECTOR_H
#define POLYGONINTERSECTOR_H

#include "mgp.h"
#include <QList>
#include <QPair>

MGP_BEGIN_NAMESPACE

class PolygonIntersector
{
public:
    static PolygonIntersector &instance();

    void setPolygons(const Polygons &polygons);
    QList<QPair<int, Polygons> > intersection(const Polygons &intersectors) const;

private:
    PolygonIntersector();
    Polygons polygons_;
};

// --- END classes --------------------------------------------------

MGP_END_NAMESPACE

#endif // POLYGONINTERSECTOR_H
