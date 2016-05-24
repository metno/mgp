#include "polygonintersector.h"
#include "mgpmath.h"

MGP_BEGIN_NAMESPACE

PolygonIntersector &PolygonIntersector::instance()
{
    static PolygonIntersector pi;
    return pi;
}

PolygonIntersector::PolygonIntersector()
{
}

void PolygonIntersector::setPolygons(const Polygons &polygons)
{
    polygons_ = polygons; // later, the polygons should be represented in a tree structure (quadtree, BSP tree etc.) in order to reduce
                          // the complexity of intersection() from O(n) to O(log n) ... TBD
}

QList<QPair<int, Polygons> > PolygonIntersector::intersection(const Polygons &intersectors) const
{
    QList<QPair<int, Polygons> > isct;

    if (!polygons_)
        return isct;

    // trivial O(n) algorithm
    for (int i = 0; i < polygons_->size(); ++i) {
        Polygons ipolys = Polygons(new QVector<Polygon>());
        for (int j = 0; j < intersectors->size(); ++j) {
            const Polygons ipolys2 = math::polygonIntersection(intersectors->at(j), polygons_->at(i));
            if (!ipolys2->isEmpty())
                *ipolys += *ipolys2;
        }
        if (!ipolys->isEmpty()) {
            isct.append(qMakePair(i, ipolys));
        }
    }

    return isct;
}

MGP_END_NAMESPACE
