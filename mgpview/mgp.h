#ifndef MGP_H
#define MGP_H

#include <QSharedPointer>
#include <QVector>
#include <QPair>
#include <QList>
#include <QString>

#define MGP_BEGIN_NAMESPACE namespace mgp {
#define MGP_END_NAMESPACE }

MGP_BEGIN_NAMESPACE

// --- BEGIN class declarations --------------------------------------------------

typedef QSharedPointer<QVector<QPair<double, double> > > PointVector;
typedef QSharedPointer<QVector<PointVector> > PointVectors;

class FilterBase
{
public:
    virtual ~FilterBase() {}
protected:
    virtual PointVectors apply(const PointVector &) const = 0; // ### maybe the underscore can be left out?
};


class PolygonFilter : public FilterBase
{
public:
    void setValue(const PointVector &);
private:
    PointVector polygon_;
};

class IntersectionFilter : public PolygonFilter
{
private:
    virtual PointVectors apply(const PointVector &) const;
};

//class UnionFilter : public PolygonFilter
//{
//private:
//    virtual PointVectors apply(const PointVector &) const;
//};


class LineFilter : public FilterBase
{
    // function common to line filters to go here ... TBD
};

class LonOrLatFilter : public LineFilter
{
public:
    void setValue(double);
    double value() const;
protected:
    LonOrLatFilter(double);
    double value_;
    virtual bool rejected(const QPair<double, double> &) const = 0;
};

class LonFilter : public LonOrLatFilter
{
protected:
    LonFilter(double);
private:
    virtual PointVectors apply(const PointVector &) const;
};

class EOfFilter : public LonFilter
{
public:
    EOfFilter(double = 0.0);
private:
    virtual bool rejected(const QPair<double, double> &) const;
};

//class WOfFilter : public LonFilter
//{
//private:
//    virtual bool rejected(const QPair<double, double> &) const;
//};

//class NOfFilter : public LonFilter
//{
//private:
//    virtual bool rejected(const QPair<double, double> &) const;
//};

//class SOfFilter : public LonFilter
//{
//private:
//    virtual bool rejected(const QPair<double, double> &) const;
//};

// --- END class declarations --------------------------------------------------


typedef QSharedPointer<FilterBase> Filter;
typedef QSharedPointer<QList<Filter> > Filters;


// --- BEGIN global functions --------------------------------------------------

// Returns the list of polygons that results from applying a filter sequence to a list of polygons.
PointVectors applyFilters(const PointVectors &, const Filters &);

// Returns the list of polygons that results from applying a filter sequence to a polygon.
PointVectors applyFilters(const PointVector &, const Filters &);

// Returns the canonical SIGMET/AIRMET expression that corresponds to a filter sequence.
QString xmetExprFromFilters(const Filters &);

// Returns the filter sequence that corresponds to a SIGMET/AIRMET expression.
// Optional out parameters (passed if non-null):
//   1: The list of matched ranges.
//   2: The list of incomplete ranges and reasons.
Filters filtersFromXmetExpr(const QString &, QList<QPair<int, int> > * = 0, QList<QPair<QPair<int, int>, QString> > * = 0);

// --- END global functions --------------------------------------------------

MGP_END_NAMESPACE

#endif // MGP_H
