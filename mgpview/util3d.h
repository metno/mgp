#ifndef UTIL3D_H
#define UTIL3D_H

//#include <MeCommon.h>
#include <math.h>


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
    /**
     * Rotates around an arbitrary axis.
     * @exception MathError The axis norm is too small.
     */
    void loadRotateXYZ(double theta, double x, double y, double z);
    /**
     * Rotates y- and x- unit axes into the specified target and up vector
     * respectively.
     */
    void loadRotateTgtUp(
	double tgt_x, double tgt_y, double tgt_z,
	double  up_x, double  up_y, double  up_z);

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
    static void normalize(double& x, double& y);
    static void normalize(double& x, double& y, double& z);
    static double angle(double x, double y);
    static double min(double a, double b) {return a < b ? a : b;}
    static double max(double a, double b) {return a > b ? a : b;}
    static double distance(double lon1, double lat1, double lon2, double lat2);

    // OBSOLETE:
    static double* sphericalToCartesian(
	double radius, double phi, double theta);
    static void cartesianToSpherical(
	double x, double y, double z, double& phi, double& theta);
    static void computeCamera(
	double radius, double phi_eye, double theta_eye, double alt_eye,
	double phi_tgt, double theta_tgt, double alpha, _4DPoint& eye,
	_4DPoint& tgt_alpha, _4DPoint& up);

    static void sphericalToCartesian(
	double radius, double phi, double theta, double& x, double& y,
	double& z);
    static bool raySphereIntersect(
	double px, double py, double pz, double rx, double ry, double rz,
	double cx, double cy, double cz, double r, double& x, double& y,
	double& z);
    static void computeLatLon(
	double x, double y, double z, double& lat, double& lon);
    static double computeRotationTowardsEye(
	double lat, double lon, double eye_x, double eye_y, double eye_z);
    static double computeRotationTowardsEye(
	double x, double y, double z, double eye_x, double eye_y, double eye_z);
};


class _3DPoint
{
public:
    _3DPoint();
    _3DPoint(const _3DPoint&);
    _3DPoint(double x, double y, double z);
    void setPoint(double x, double y, double z)
    {c_[0] = x; c_[1] = y; c_[2] = z;}
    void setPoint(double* c) {c_[0] = c[0]; c_[1] = c[1]; c_[2] = c[2];}
    double* getPoint() {return c_;}
    double x() const {return c_[0];}
    double y() const {return c_[1];}
    double z() const {return c_[2];}
    void print(char lead[]) const;
private:
    double c_[3];
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
