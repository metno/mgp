#include "mgp.h"

MGP_BEGIN_NAMESPACE

PointVectors apply(const PointVectors &polygons, const QList<FilterBase *> &filters)
{
    // for now
    Q_UNUSED(filters);
    return polygons;
}

LonOrLatFilter::LonOrLatFilter(double value)
    : value_(value)
{
}

void LonOrLatFilter::setValue(double value)
{
    value_ = value;
}

double LonOrLatFilter::value() const
{
    return value_;
}

LonFilter::LonFilter(double value)
    : LonOrLatFilter(value)
{
}

PointVectors LonFilter::apply(const PointVector &inPoly) const
{
    PointVectors outPolys = PointVectors(new QVector<PointVector>());

    // for now
    Q_UNUSED(inPoly);
    return outPolys;
}

EOfFilter::EOfFilter(double value)
    : LonFilter(value)
{
}

bool EOfFilter::rejected(const QPair<double, double> &point) const
{
    return point.first < value_;
}

PointVectors applyFilters(const PointVectors &polygons, const Filters &filters)
{
    // ### for now
    Q_UNUSED(filters);
    return polygons;
}

PointVectors applyFilters(const PointVector &polygon, const Filters &filters)
{
    PointVectors polygons = PointVectors(new QVector<PointVector>());
    polygons->append(polygon);
    return applyFilters(polygons, filters);
}

QString xmetExprFromFilters(const Filters &filters)
{
    // ### for now
    Q_UNUSED(filters);
    return QString();
}

Filters filtersFromXmetExpr(const QString &, QList<QPair<int, int> > *matchedRanges, QList<QPair<QPair<int, int>, QString> > *incompleteRanges)
{
    // ### for now
    Q_UNUSED(matchedRanges);
    Q_UNUSED(incompleteRanges);
    return Filters();
}

MGP_END_NAMESPACE
