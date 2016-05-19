#ifndef MGPMATH_H
#define MGPMATH_H

#include "mgp.h"

#define MGPMATH_BEGIN_NAMESPACE namespace math {
#define MGPMATH_END_NAMESPACE }

MGP_BEGIN_NAMESPACE
MGPMATH_BEGIN_NAMESPACE

// --- BEGIN classes --------------------------------------------------

class _4x4Matrix
{
public:
    _4x4Matrix();
    void set(int i, int j, double val) {c_[i][j] = val;};
    double get(int i, int j) const {return c_[i][j];};
    // Right-multiplies this matrix with m (this * m) storing the result in
    // this matrix:
    void mulMat(const _4x4Matrix& m);
    // Left-multiplies this matrix with m (m * this) storing the result in
    // this matrix:
    void mulMatLeft(const _4x4Matrix& m);
    void loadIdentity();

    // Rotates around a primary axis
    void loadRotateX(double theta);
    void loadRotateY(double theta);
    void loadRotateZ(double theta);

    void loadTranslate(double x, double y, double z);

    void print(char lead[]) const;
private:
    double c_[4][4];
};

class _4DPoint
{
public:
    _4DPoint();
    _4DPoint(const _4DPoint&);
    _4DPoint(double x, double y, double z);
    void set(int i, double val) {c_[i] = val;}
    void set(double x, double y, double z)
        {c_[0] = x; c_[1] = y; c_[2] = z; c_[3] = 1;}
    double get(int i) const {return c_[i];}
    double x() const {return c_[0];}
    double y() const {return c_[1];}
    double z() const {return c_[2];}
    void mulMatPoint(const _4x4Matrix& m);
    double dot(const _4DPoint& p);
    void cross(const _4DPoint& p);
    void rotate(const _4DPoint& p, const double alpha);
    void normalize();
    void scale(double fact);
    void add(const _4DPoint& p);
    void subtract(const _4DPoint& p);
    void print(char lead[]) const;
private:
    double c_[4];
};

class _3DPoint
{
public:
    _3DPoint();
    _3DPoint(const _3DPoint &);
    _3DPoint & operator=(const _3DPoint &);
    _3DPoint(double x, double y, double z);
    void setPoint(double x, double y, double z) { c_[0] = x; c_[1] = y; c_[2] = z; }
    void setPoint(double* c) { c_[0] = c[0]; c_[1] = c[1]; c_[2] = c[2]; }
    double* getPoint() { return c_; }
    double x() const { return c_[0]; }
    double y() const { return c_[1]; }
    double z() const { return c_[2]; }
    double norm() const;
    static _3DPoint fromSpherical(double lon, double lat);
    static _3DPoint cross(const _3DPoint &p1, const _3DPoint &p2);
    static double dot(const _3DPoint &p1, const _3DPoint &p2);
    void normalize();
    static _3DPoint normalized(const _3DPoint &p);
private:
    double c_[3];
};

class Math
{
public:
    static double sqr(double x) {return x * x;}
    static void normalize(double &x, double &y);
    static void normalize(double &x, double &y, double &z);
    static double norm(double x, double y);
    static double norm(double x, double y, double z);
    static double angle(double x, double y);

    /**
     * Returns the spherical distance (i.e. along the great circle on the unit sphere) between two points.
     *
     * @param   p1 - first point.
     * @param   p2 - second point.
     * @returns Distance between p1 and p2 on unit sphere.
     */
    static double distance(const Point &p1, const Point &p2);

    // Returns the initial bearing from p1 to p2 in radians from north.
    static double bearingBetween(const Point &p1, const Point &p2);

    static void sphericalToCartesian(double radius, double phi, double theta, double &x, double &y,	double &z);
    static void cartesianToSpherical(double x, double y, double z, double &phi, double &theta);
    static bool raySphereIntersect(
            double px, double py, double pz, double rx, double ry, double rz,
            double cx, double cy, double cz, double r, double &x, double &y, double &z);
    static void computeLatLon(double x, double y, double z, double &lat, double &lon);
};

// --- END classes --------------------------------------------------

// --- BEGIN global functions --------------------------------------------------

// Returns true iff the great circle arc from p1 to p2 intersects the great circle arc from p3 to p4.
// The intersection point closest to p1 and p2 is returned in isctPoint.
// NOTE: If the two great circles lie (approximately) in the same plane, the function returns false (although there are infinitely many
// intersections!).
bool greatCircleArcsIntersect(const Point &p1, const Point &p2, const Point &p3, const Point &p4, Point *isctPoint = 0);

// Returns true iff the great circle through p1 and p2 intersects the great circle through p3 and p4.
// The two intersection points are returned in isctPoint1 and isctPoint2.
// NOTE: If the two great circles lie (approximately) in the same plane, the function returns false (although there are infinitely many
// intersections!).
bool greatCirclesIntersect(const Point &p1, const Point &p2, const Point &p3, const Point &p4, Point *isctPoint1, Point *isctPoint2);

// Returns the number of intersections (0, 1 or 2) between the great circle arc from p1 to p2 and a great circle through p3 and p4.
// Up to two intersection points are returned in isctPoint1 and isctPoint2.
// NOTE: If the two great circles lie (approximately) in the same plane, the function returns 0 (although there are infinitely many
// intersections!).
int greatCircleArcIntersectsGreatCircle(const Point &p1, const Point &p2, const Point &p3, const Point &p4, Point *isctPoint1, Point *isctPoint2);

// Returns signed distance from p0 to the great circle arc from p1 to p2.
// The return value is negative iff p0 is considered to be to the left of the arc.
double crossTrackDistanceToGreatCircle(const Point &p0, const Point &p1, const Point &p2);

// Returns the points of the great circle through p1 and p2. If segmentOnly is true, only the part of the circle between p1 and p2 is returned.
QVector<_3DPoint> greatCirclePoints(const QPair<double, double> &p1, const QPair<double, double> &p2, int nSegments, bool segmentOnly = true);

// Returns true iff a point is considered inside a polygon.
bool pointInPolygon(const Point &point, const Polygon &polygon);

// Returns the polygons that form the intersection of two polygons.
Polygons polygonIntersection(const Polygon &subject, const Polygon &clip);

// Returns the points (0, 1 or 2) where lat intersects the great circle arc from p1 to p2.
// If two intersections are found, the one closest to p1 appears first in the result vector.
QVector<Point> latitudeIntersections(const Point &p1, const Point &p2, double lat);

// Returns the points at latitude lat from longitude lon1 to lon2. The two input longitudes are included iff inclusive is true.
Polygon latitudeArcPoints(double lat, double lon1, double lon2, bool eastwards, int nSegments, bool inclusive);

// Returns a reversed copy of a polygon.
Polygon reversed(const Polygon &polygon);

// Returns true iff a polygon is oriented clockwise.
bool isClockwise(const Polygon &polygon);

// --- END global functions --------------------------------------------------

MGPMATH_END_NAMESPACE
MGP_END_NAMESPACE

#endif // MGPMATH_H
