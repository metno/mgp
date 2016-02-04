#ifndef UTIL3D_H
#define UTIL3D_H

//#include <MeCommon.h>
#include <math.h>
#include <QVector>
#include <QPair>

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

    void print(char lead[]) const;
private:
    double c_[3];
};

#define DEG2RAD(d) ((d) / 180.0) * M_PI
#define RAD2DEG(r) ((r) / M_PI) * 180

class Math
{
public:
    static bool equal(double a, double b)
	{return fabs(a - b) < 1e-8;}
    static double sqr(double x) {return x * x;}
    static double norm(double x, double y)
	{return sqrt(x * x + y * y);}
    static double norm(double x, double y, double z)
	{return sqrt(x * x + y * y + z * z);}
    static double norm(double x[3])
	{return sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);}
    static void normalize(double &x, double &y);
    static void normalize(double &x, double &y, double &z);
    static double angle(double x, double y);
    static double min(double a, double b) {return a < b ? a : b;}
    static double max(double a, double b) {return a > b ? a : b;}
    static double distance(double lon1, double lat1, double lon2, double lat2);
    static QVector<_3DPoint> getGreatCirclePoints(double lon1, double lat1, double lon2, double lat2, int nSegments);

    static double distanceBetween(double lon0, double lat0, double lon1, double lat1, double radius = 1.0);
    static double bearingBetween(double lon0, double lat0, double lon1, double lat1);
    static double crossTrackDistanceToGreatCircle(double lon0, double lat0, double lon1, double lat1, double lon2, double lat2, double radius = 1.0);

    static bool intersectsLatitude(const QPair<double, double> &p1, const QPair<double, double> &p2, double lat, QPair<double, double> *isctPoint);
    static bool greatCircleArcsIntersect(
            const QPair<double, double> &p1, const QPair<double, double> &p2, double lon1, double lat1, double lon2, double lat2,
            QPair<double, double> *isctPoint);

    static double *sphericalToCartesian(double radius, double phi, double theta);
    static void sphericalToCartesian(double radius, double phi, double theta, double &x, double &y,	double &z);
    static void cartesianToSpherical(double x, double y, double z, double &phi, double &theta);
    static void computeCamera(
            double radius, double phi_eye, double theta_eye, double alt_eye, double phi_tgt, double theta_tgt, double alpha,
            _4DPoint &eye, _4DPoint &tgt_alpha, _4DPoint &up);
    static bool raySphereIntersect(
	double px, double py, double pz, double rx, double ry, double rz,
    double cx, double cy, double cz, double r, double &x, double &y, double &z);
    static void computeLatLon(double x, double y, double z, double &lat, double &lon);
};


class DynPoly
{
public:
    void setSize(int size) {size_ = size; id_ = new int[size];}
    void setId(int i, int val) {id_[i] = val;}
    int getId(int i) {return id_[i];}
    int getSize() {return size_;}
private:
    int size_, * id_;
};



#endif // UTIL3D_H
