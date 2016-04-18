#ifndef MGP_H
#define MGP_H

#include <QSharedPointer>
#include <QVector>
#include <QPair>
#include <QList>
#include <QString>
#include <QVariant>
#include <QTextEdit>
#include <QBitArray>
#include <math.h>

#define MGP_BEGIN_NAMESPACE namespace mgp {
#define MGP_END_NAMESPACE }

 /*!
 * \addtogroup mgp mgp
 * This module constitutes the main API.
 * @{
 */

/// \cond
MGP_BEGIN_NAMESPACE
/// \endcond

/**
 * The Point type represents a <a href="https://en.wikipedia.org/wiki/Spherical_coordinate_system">spherical coordinate</a> where
 * the following is assumed by default:
 * <ul>
 * <li> the first component is the longitude (azimuth) in the range [-M_PI, M_PI] </li>
 * <li> the second component is the latitude (inclination) in the range [-M_PI_2, M_PI_2] </li>
 * <li> the radius is 1 (i.e. the point is defined on the unit sphere) </li>
 * </ul>
 * (M_PI and M_PI_2 are defined in /usr/include/math.h)
 *
 * <pre>
 * Examples:
 *   South pole: qMakePair(anyLon, -M_PI_2) (i.e. qMakePair(anyLon,               (-90 / 90) * M_PI_2))
 *   North pole: qMakePair(anyLon,  M_PI_2) (i.e. qMakePair(anyLon,               ( 90 / 90) * M_PI_2))
 *   Oslo:       qMakePair(0.1876, 1.0463)  (i.e. qMakePair((10.75 / 180) * M_PI, (59.95 / 90) * M_PI_2))
 * </pre>
*/
typedef QPair<double, double> Point;
typedef QSharedPointer<QVector<Point> > Polygon;
typedef QSharedPointer<QVector<Polygon> > Polygons;

#define DEG2RAD(d) ((d) / 180.0) * M_PI
#define RAD2DEG(r) ((r) / M_PI) * 180


// --- BEGIN classes --------------------------------------------------

//! Abstract interface for filters.
class FilterBase
{
public:
    virtual ~FilterBase() {}

    enum Type {
        Unsupported,
        WI,
        E_OF, W_OF, N_OF, S_OF,
        E_OF_LINE, W_OF_LINE, N_OF_LINE, S_OF_LINE,
        NE_OF_LINE, NW_OF_LINE, SE_OF_LINE, SW_OF_LINE
    };
    virtual Type type() const = 0;

    /** Sets the filter value from a QVariant. */
    virtual void setFromVariant(const QVariant &var) = 0;

    /** Returns the filter value as a QVariant. */
    virtual QVariant toVariant() const = 0;

    /** Returns true iff the filter is considered to be in a valid state. */
    virtual bool isValid() const = 0;

    /** Applies the filter.
     * \param inPoly The input polygon (assumed to contain at least three points).
     * \return Zero or more polygons resulting from applying the filter to \c inPoly.
     */
    virtual Polygons apply(const Polygon &inPoly) const = 0;

    /** Returns all intersection points between the filter and the given polygon. */
    virtual QVector<Point> intersections(const Polygon &) const = 0;

    /** Sets the filter state from a SIGMET/AIRMET expression. (### more documentation here!) */
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *) = 0;

    /** Returns the canonical SIGMET/AIRMET expression corresponding to the filter state. */
    virtual QString xmetExpr() const = 0;

    /**
     * Returns true iff the given point is rejected by the filter. A point that is rejected would not be included in any polygon returned
     * by apply() (regardless of the shape of the input polygon), and vice versa: a point that is not rejected will always be included in one of
     * those polygons.
     */
    virtual bool rejected(const Point &) const = 0;
};

//! This is the base class for filters that represent regions explicitly defined as closed polygons.
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
    virtual void setFromVariant(const QVariant &);
    virtual QVariant toVariant() const;

    /** Returns true iff the polygon consists of at least three points. */
    virtual bool isValid() const;
};

//! This filter clips away regions outside a specific closed polygon.
class WithinFilter : public PolygonFilter
{
public:
    /** Constructs an object with an empty polygon. */
    WithinFilter();

    /** Constructs an object with a given polygon. */
    WithinFilter(const Polygon &);

private:
    virtual Type type() const { return WI; }
    virtual Polygons apply(const Polygon &) const;
    virtual QVector<Point> intersections(const Polygon &inPoly) const;
    virtual bool rejected(const Point &) const;
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *);
    virtual QString xmetExpr() const;
};

//! This filter is currently not implemented, but eventually it is intended to generate the union of two explicitly defined polygons.
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

//! This is the base class for filters that represent regions that are implicitly defined as being on one or the other side of a line.
class LineFilter : public FilterBase
{
public:
    QString directionName() const;

protected:
    // Returns true and intersection point iff filter intersects great circle arc between given two points. If the filter intersects
    // the arc twice, the intersection closest to the first endpoint is returned.
    virtual bool intersects(const Point &, const Point &, Point *) const = 0;

private:
    virtual Polygons apply(const Polygon &) const;
    virtual QVector<Point> intersections(const Polygon &inPoly) const;
};

//! This filter clips away regions on one or the other side of a specific longitude or latitude.
class LonOrLatFilter : public LineFilter
{
public:
    void setValue(double);
    double value() const;
    bool isLonFilter() const;
protected:
    LonOrLatFilter(double);
    double value_;
private:
    virtual void setFromVariant(const QVariant &);
    virtual QVariant toVariant() const;
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *);
    virtual QString xmetExpr() const;
};

//! This filter clips away regions on one or the other side of a specific longitude.
class LonFilter : public LonOrLatFilter
{
public:
    LonFilter(double);
private:
    virtual bool intersects(const Point &, const Point &, Point *) const;
};

//! This filter clips away regions that are not east of a specific longitude.
class EOfFilter : public LonFilter
{
public:
    EOfFilter(double = DEG2RAD(-8));
private:
    virtual Type type() const { return E_OF; }
    virtual bool isValid() const;
    virtual bool rejected(const Point &) const;
};

//! This filter clips away regions that are not west of a specific longitude.
class WOfFilter : public LonFilter
{
public:
    WOfFilter(double = DEG2RAD(15));
private:
    virtual Type type() const { return W_OF; }
    virtual bool isValid() const;
    virtual bool rejected(const Point &) const;
};

//! This filter clips away regions on one or the other side of a specific latitude.
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

//! This filter clips away regions that are not north of a specific latitude.
class NOfFilter : public LatFilter
{
public:
    NOfFilter(double = DEG2RAD(60));
private:
    virtual Type type() const { return N_OF; }
    virtual bool isValid() const;
    virtual bool rejected(const Point &) const;
};

//! This filter clips away regions that are not south of a specific latitude.
class SOfFilter : public LatFilter
{
public:
    SOfFilter(double = DEG2RAD(70));
private:
    virtual Type type() const { return S_OF; }
    virtual bool isValid() const;
    virtual bool rejected(const Point &) const;
};

//! This filter clips away regions on one or the other side of a specific line defined by two (lon, lat) points.
class FreeLineFilter : public LineFilter
{
public:
    void setLine(const Point &, const Point &);
    void setLine(const QPair<Point, Point> &);
    void setPoint1(const Point &);
    void setPoint2(const Point &);
    QPair<Point, Point> line() const { return line_; }
    Point point1() const { return line_.first; }
    Point point2() const { return line_.second; }
    double lon1() const { return line_.first.first; }
    double lat1() const { return line_.first.second; }
    double lon2() const { return line_.second.first; }
    double lat2() const { return line_.second.second; }
protected:
    FreeLineFilter(const QPair<Point, Point> &);
    QPair<Point, Point> line_;
private:
    virtual void setFromVariant(const QVariant &);
    virtual QVariant toVariant() const;
    virtual bool isValid() const;
    virtual bool intersects(const Point &, const Point &, Point *) const;
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *);
    virtual QString xmetExpr() const;
    virtual bool rejected(const Point &) const;
};

//! This filter clips away regions that are not east of a specific line.
class EOfLineFilter : public FreeLineFilter
{
public:
    EOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(5), DEG2RAD(55)), qMakePair(DEG2RAD(7), DEG2RAD(70))));
private:
    virtual Type type() const { return E_OF_LINE; }
};

//! This filter clips away regions that are not west of a specific line.
class WOfLineFilter : public FreeLineFilter
{
public:
    WOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(15), DEG2RAD(55)), qMakePair(DEG2RAD(17), DEG2RAD(70))));
private:
    virtual Type type() const { return W_OF_LINE; }
};

//! This filter clips away regions that are not north of a specific line.
class NOfLineFilter : public FreeLineFilter
{
public:
    NOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(-5), DEG2RAD(62)), qMakePair(DEG2RAD(15), DEG2RAD(64))));
private:
    virtual Type type() const { return N_OF_LINE; }
};

//! This filter clips away regions that are not south of a specific line.
class SOfLineFilter : public FreeLineFilter
{
public:
    SOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(-5), DEG2RAD(72)), qMakePair(DEG2RAD(15), DEG2RAD(74))));
private:
    virtual Type type() const { return S_OF_LINE; }
};

//! This filter clips away regions that are not northeast of a specific line.
class NEOfLineFilter : public FreeLineFilter
{
public:
    NEOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(3), DEG2RAD(53)), qMakePair(DEG2RAD(8), DEG2RAD(72))));
private:
    virtual Type type() const { return NE_OF_LINE; }
};

//! This filter clips away regions that are not northwest of a specific line.
class NWOfLineFilter : public FreeLineFilter
{
public:
    NWOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(13), DEG2RAD(53)), qMakePair(DEG2RAD(18), DEG2RAD(72))));
private:
    virtual Type type() const { return NW_OF_LINE; }
};

//! This filter clips away regions that are not southeast of a specific line.
class SEOfLineFilter : public FreeLineFilter
{
public:
    SEOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(-3), DEG2RAD(63)), qMakePair(DEG2RAD(18), DEG2RAD(64))));
private:
    virtual Type type() const { return SE_OF_LINE; }
};

//! This filter clips away regions that are not southwest of a specific line.
class SWOfLineFilter : public FreeLineFilter
{
public:
    SWOfLineFilter(const QPair<Point, Point> & = qMakePair(qMakePair(DEG2RAD(-3), DEG2RAD(73)), qMakePair(DEG2RAD(18), DEG2RAD(74))));
private:
    virtual Type type() const { return SW_OF_LINE; }
};


typedef QSharedPointer<FilterBase> Filter;
typedef QSharedPointer<QList<Filter> > Filters;


class FIR
{
public:
    static FIR &instance();

    /**
     * Supported Flight Information Regions
     */
    enum Code {
        Unsupported,
        ENOR, // NORWAY
        ENOB // BODO OCEANIC
    };

    /**
     * Converts a FIR code to a polygon.
     * @param[in] fir FIR code.
     * @return a non-empty polygon for a supported FIR code, otherwise an empty polygon.
     */
    Polygon polygon(Code fir) const;

    /**
     * Returns the first supported FIR found in a SIGMET/AIRMET area expression.
     * @param[in] expr SIGMET/AIRMET area expression.
     * @return The code for the first supported FIR found in the expression, otherwise the code for an unsupported FIR.
     */
    static Code firFromXmetExpr(const QString &expr);

private:
    FIR();
    QHash<Code, Polygon> fir_;
};


//! This class extends a QTextEdit with validation and highlighting of a SIGMET/AIRMET area expression.
class XMETAreaEdit : public QTextEdit
{
public:
    XMETAreaEdit(QWidget *parent = 0);
    XMETAreaEdit(const QString &text, QWidget *parent = 0);

    /**
     * Updates filters and highlighting.
     * \return True iff each non-whitespace character is part of a matched range.
     */
    bool update();

    /**
     * Extracts the complete filters found in the text
     * \return The sequence of complete filters ordered by occurrence. If a given filter type (like 'S OF ...') appears more than
     * once, only the first occurrence is returned.
     */
    Filters filters();

    /**
     * Extracts the first FIR found in the text
     * \return The code for the first supported FIR found in the text, otherwise the code for an unsupported FIR.
     */
    FIR::Code fir();

private:
    void init();
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    void resetHighlighting();
    void addMatchedRange(const QPair<int, int> &);
    void addIncompleteRange(const QPair<int, int> &, const QString &);
    void showHighlighting();
    QBitArray matched_;
    QBitArray incomplete_;
    QHash<int, QString> reason_;
    Filters filters_;
    FIR::Code fir_;
};

// --- END classes --------------------------------------------------


// --- BEGIN global functions --------------------------------------------------

/**
 * Applies a filter sequence to a set of polygons.
 *
 * \param[in] polygons Set of zero or more polygons.
 * \param[in] filters  Sequence of zero or more filters.
 * \return The list of polygons that results from applying \c filters to \c polygons.
 */
Polygons applyFilters(const Polygons &polygons, const Filters &filters);

/**
 * Applies a filter sequence to a single polygon.
 *
 * \note This is an overloaded function.
 * \param[in] polygon Polygon.
 * \param[in] filters Sequence of zero or more filters.
 * \return The list of polygons that results from applying \c filters to \c polygon.
 */
Polygons applyFilters(const Polygon &polygon, const Filters &filters);

/**
 * Converts a filter sequence to a SIGMET/AIRMET area expression.
 *
 * \param[in] filters Sequence of zero or more filters.
 * \return The canonical area part of a SIGMET/AIRMET expression that corresponds to \c filters.
 */
QString xmetExprFromFilters(const Filters &filters);

/**
 * Converts a SIGMET/AIRMET area expression to a filter sequence.
 *
 * \param[in]  expr             SIGMET/AIRMET area expression.
 * \param[out] matchedRanges    Sequence of string position ranges of each fully matched filter in \c expr.
 * \param[out] incompleteRanges Sequence of string position ranges of each incomplete (half matched) filter in \c expr along with the reasons why they didn't match.
 * \return The filter sequence representing fully matched filters in the order of appearance in \c expr.
 */
Filters filtersFromXmetExpr(const QString &expr, QList<QPair<int, int> > *matchedRanges = 0, QList<QPair<QPair<int, int>, QString> > *incompleteRanges = 0);

// --- END global functions --------------------------------------------------

MGP_END_NAMESPACE
/*! @} end of Doxygen group mgp */

#endif // MGP_H
