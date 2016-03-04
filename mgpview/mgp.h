#ifndef MGP_H
#define MGP_H

#include <QSharedPointer>
#include <QVector>
#include <QPair>
#include <QList>
#include <QString>
#include <math.h>

#define MGP_BEGIN_NAMESPACE namespace mgp {
#define MGP_END_NAMESPACE }

MGP_BEGIN_NAMESPACE

// The Polygon type represents a spherical polygon, i.e. a set of three or more points in (lon, lat) format where
// -M_PI <= lon <= M_PI and -M_PI_2 <= lat <= M_PI_2 (M_PI and M_PI_2 are defined in /usr/include/math.h).
// Examples:
//   South pole: qMakePair(anyLon, -M_PI_2) (i.e. qMakePair(anyLon,               (-90 / 90) * M_PI_2))
//   North pole: qMakePair(anyLon,  M_PI_2) (i.e. qMakePair(anyLon,               ( 90 / 90) * M_PI_2))
//   Oslo:       qMakePair(0.1876, 1.0463)  (i.e. qMakePair((10.75 / 180) * M_PI, (59.95 / 90) * M_PI_2))
typedef QPair<double, double> Point;
typedef QSharedPointer<QVector<Point> > Polygon;
typedef QSharedPointer<QVector<Polygon> > Polygons;

#define DEG2RAD(d) ((d) / 180.0) * M_PI
#define RAD2DEG(r) ((r) / M_PI) * 180

// --- BEGIN classes --------------------------------------------------

class FilterBase
{
public:
    virtual ~FilterBase() {}

    // Returns true iff the filter is considered to be in a valid state.
    virtual bool isValid() const = 0;

    // Returns the polygons resulting from applying the filter to the given polygon.
    virtual Polygons apply(const Polygon &) const = 0;

    // Returns all intersection points between the filter and the given polygon.
    virtual QVector<Point> intersections(const Polygon &) const = 0;

    // Sets the filter state from a SIGMET/AIRMET expression. (### more documentation here!)
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *) = 0;

    // Returns the canonical SIGMET/AIRMET expression corresponding to the filter state.
    virtual QString xmetExpr() const = 0;

protected:
    enum Type {
        Unsupported,
        WI,
        E_OF, W_OF, N_OF, S_OF,
        E_OF_LINE, W_OF_LINE, N_OF_LINE, S_OF_LINE,
        NE_OF_LINE, NW_OF_LINE, SE_OF_LINE, SW_OF_LINE
    };
    virtual Type type() const = 0;

    // Returns true iff the given point is rejected by the filter. A point that is rejected would not be included in any polygon returned
    // by apply() (regardless of the shape of the input polygon), and vice versa: a point that is not rejected will always be included in one of
    // those polygons.
    virtual bool rejected(const Point &) const = 0;
};

class PolygonFilter : public FilterBase
{
public:
    void setPolygon(const Polygon &);
    Polygon polygon() const;
protected:
    PolygonFilter();
    PolygonFilter(const Polygon &);
    Polygon polygon_;
private:
    virtual bool isValid() const;
};

class WithinFilter : public PolygonFilter // NOTE: this could also be named IntersectionFilter
{
public:
    WithinFilter();
    WithinFilter(const Polygon &);
private:
    virtual Type type() const { return WI; }
    virtual Polygons apply(const Polygon &) const;
    virtual QVector<Point> intersections(const Polygon &inPoly) const;
    virtual bool rejected(const Point &) const;
    virtual QString xmetExpr() const;
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *);
};

class UnionFilter : public PolygonFilter
{
public:
    UnionFilter();
    UnionFilter(const Polygon &);
private:
    virtual Type type() const { return Unsupported; } // ###
    virtual Polygons apply(const Polygon &) const;
    virtual QVector<Point> intersections(const Polygon &inPoly) const;
    virtual bool rejected(const Point &) const;
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *);
    virtual QString xmetExpr() const;
};

class LineFilter : public FilterBase
{
protected:
    // Returns true and intersection point iff filter intersects great circle arc between given two points. If the filter intersects
    // the arc twice, the intersection closest to the first endpoint is returned.
    virtual bool intersects(const Point &, const Point &, Point *) const = 0;

    QString directionName() const;

private:
    virtual Polygons apply(const Polygon &) const;
    virtual QVector<Point> intersections(const Polygon &inPoly) const;
};

class LonOrLatFilter : public LineFilter
{
public:
    void setValue(double);
    double value() const;
protected:
    LonOrLatFilter(double);
    double value_;
private:
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *);
    virtual QString xmetExpr() const;
    bool isLonFilter() const;
};

class LonFilter : public LonOrLatFilter
{
public:
    LonFilter(double);
private:
    virtual bool intersects(const Point &, const Point &, Point *) const;
};

class EOfFilter : public LonFilter
{
public:
    EOfFilter(double = DEG2RAD(-8));
private:
    virtual Type type() const { return E_OF; }
    virtual bool isValid() const;
    virtual bool rejected(const Point &) const;
};

class WOfFilter : public LonFilter
{
public:
    WOfFilter(double = DEG2RAD(15));
private:
    virtual Type type() const { return W_OF; }
    virtual bool isValid() const;
    virtual bool rejected(const Point &) const;
};

class LatFilter : public LonOrLatFilter
{
public:
    LatFilter(double);
private:
    virtual Polygons apply(const Polygon &) const;
    virtual QVector<Point> intersections(const Polygon &inPoly) const;
    virtual bool intersects(const Point &, const Point &, Point *) const;
    bool isNOfFilter() const;
};

class NOfFilter : public LatFilter
{
public:
    NOfFilter(double = DEG2RAD(60));
private:
    virtual Type type() const { return N_OF; }
    virtual bool isValid() const;
    virtual bool rejected(const Point &) const;
};

class SOfFilter : public LatFilter
{
public:
    SOfFilter(double = DEG2RAD(70));
private:
    virtual Type type() const { return S_OF; }
    virtual bool isValid() const;
    virtual bool rejected(const Point &) const;
};

class FreeLineFilter : public LineFilter
{
public:
    void setLine(const Point &, const Point &);
    void setLine(const QPair<Point, Point> &);
    QPair<Point, Point> line() const;
protected:
    FreeLineFilter(const QPair<Point, Point> &);
    QPair<Point, Point> line_;
    double lon1() const { return line_.first.first; }
    double lat1() const { return line_.first.second; }
    double lon2() const { return line_.second.first; }
    double lat2() const { return line_.second.second; }
private:
    virtual bool isValid() const;
    virtual bool intersects(const Point &, const Point &, Point *) const;
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *);
    virtual QString xmetExpr() const;
    virtual bool rejected(const Point &) const;
};

class EOfLineFilter : public FreeLineFilter
{
public:
    EOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(5), DEG2RAD(55)), qMakePair(DEG2RAD(7), DEG2RAD(70))));
private:
    virtual Type type() const { return E_OF_LINE; }
};

class WOfLineFilter : public FreeLineFilter
{
public:
    WOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(15), DEG2RAD(55)), qMakePair(DEG2RAD(17), DEG2RAD(70))));
private:
    virtual Type type() const { return W_OF_LINE; }
};

class NOfLineFilter : public FreeLineFilter
{
public:
    NOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(-5), DEG2RAD(62)), qMakePair(DEG2RAD(15), DEG2RAD(64))));
private:
    virtual Type type() const { return N_OF_LINE; }
};

class SOfLineFilter : public FreeLineFilter
{
public:
    SOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(-5), DEG2RAD(72)), qMakePair(DEG2RAD(15), DEG2RAD(74))));
private:
    virtual Type type() const { return S_OF_LINE; }
};

class NEOfLineFilter : public FreeLineFilter
{
public:
    NEOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(3), DEG2RAD(53)), qMakePair(DEG2RAD(8), DEG2RAD(72))));
private:
    virtual Type type() const { return NE_OF_LINE; }
};

class NWOfLineFilter : public FreeLineFilter
{
public:
    NWOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(13), DEG2RAD(53)), qMakePair(DEG2RAD(18), DEG2RAD(72))));
private:
    virtual Type type() const { return NW_OF_LINE; }
};

class SEOfLineFilter : public FreeLineFilter
{
public:
    SEOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(-3), DEG2RAD(63)), qMakePair(DEG2RAD(18), DEG2RAD(64))));
private:
    virtual Type type() const { return SE_OF_LINE; }
};

class SWOfLineFilter : public FreeLineFilter
{
public:
    SWOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(-3), DEG2RAD(73)), qMakePair(DEG2RAD(18), DEG2RAD(74))));
private:
    virtual Type type() const { return SW_OF_LINE; }
};

// --- END classes --------------------------------------------------


typedef QSharedPointer<FilterBase> Filter;
typedef QSharedPointer<QList<Filter> > Filters;


// --- BEGIN global functions --------------------------------------------------

// Returns the list of polygons that results from applying a filter sequence to a list of polygons.
Polygons applyFilters(const Polygons &, const Filters &);

// Returns the list of polygons that results from applying a filter sequence to a polygon.
Polygons applyFilters(const Polygon &, const Filters &);

// Returns the canonical SIGMET/AIRMET expression that corresponds to a filter sequence.
QString xmetExprFromFilters(const Filters &);

// Returns the filter sequence that corresponds to a SIGMET/AIRMET expression.
// Out parameters:
//   1: The list of matched ranges.
//   2: The list of incomplete ranges and reasons.
Filters filtersFromXmetExpr(const QString &, QList<QPair<int, int> > * = 0, QList<QPair<QPair<int, int>, QString> > * = 0);

// --- END global functions --------------------------------------------------

MGP_END_NAMESPACE

#endif // MGP_H
