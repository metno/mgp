#ifndef GFXUTILS_H
#define GFXUTILS_H

#include <qgl.h>
#include "util3d.h"
#include "cartesiankeyframe.h"
#include <QList>
#include <QString>

class GfxUtils
{
public:
    static GfxUtils &instance()
	{
        static GfxUtils obj;
        return obj;
	}

    /** Draws coast contours on the earth sphere. The farther the eye
     * is from the earth surface, the more the contours are raised above the
     * surface. */
    void drawCoastContours(
	_3DPoint* eye, double min_eye_dist = 0.05 * earth_radius_,
	double max_eye_dist = 5 * earth_radius_);

    /** Draws ENOR FIR on the earth sphere. The farther the eye
     * is from the earth surface, the more the contours are raised above the
     * surface. */
    void drawENORFIR(
    _3DPoint* eye, double min_eye_dist = 0.05 * earth_radius_,
    double max_eye_dist = 5 * earth_radius_);

    /** Draws a sphere. */
    void drawSphere(
	double x, double y, double z, double radius, float r, float g, float b,
	float amb, int phi_res, int theta_res, GLenum shade_model);

    /** Draws a line. */
    void drawLine(
	double x0, double y0, double z0, double x1, double y1, double z1,
	double scale_fact, float r, float g, float b, double width = 1);

    /** Draws a cone. */
    void drawCone(
	double x0, double y0, double z0, double x1, double y1, double z1,
	double base, double length, float r, float g, float b, float amb,
	bool reverse = false);

    /** Draws a black/white camera sphere. */
    void drawCameraSphere(
	double x, double y, double z, double radius, float r, float g,
	float b);

    /** Draws a base circle in the xy-plane centered around the z-axis. */
    void drawBaseCircle(double radius, float r, float g, float b);

    /** Draws the lat/lon circles of the earth surface. The farther the eye
     * is from the earth surface, the more the circles are raised above the
     * surface. */
    void drawLatLonCircles(
	_3DPoint* eye, double min_eye_dist, double max_eye_dist);

//    /** Draws a cartesian key frame. */
//    void drawCartesianKeyFrame(
//	CartesianKeyFrame& ckf, float r, float g, float b, bool draw_eye_line,
//	bool regular, bool camera_indicator);

    static double getEarthRadius() {return earth_radius_;}

    void drawBottomString(const QString &, int, int, int, int, const QColor &, const QColor &, bool = true);

private:
    /**
     * Private constructor/destructor. (This class can only be constructed/
     * destructed by the singleton object.)
     */
    GfxUtils();
    ~GfxUtils();

    // Methods and variables relevant for coast contours.
    void createCoast();
    _3DPoint* points_;
    DynPoly* polys_;
    int n_polys_;

    // Methods and variables relevant for ENOR FIR.
    void createENORFIR();
    QList<qreal> enorLon_;
    QList<qreal> enorLat_;

    /** Earth radius in meters. */
    static const double earth_radius_;

    double computeRaise(
	_3DPoint* eye, double min_eye_dist, double max_eye_dist);
};

#endif // GFXUTILS_H
