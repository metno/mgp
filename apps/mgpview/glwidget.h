#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "controlpanel.h"
#include "mgp.h"
#include "mgpmath.h"
#include <QGLWidget>
#include <QPair>
#include <QLineF>
#include <QVariant>
#include <QList>

/**
 * This class represents a cartesian key frame: An eye point, a target vector,
 * and an up vector. The target- and up vectors are both normalized (having
 * length 1) and relative in the sense that, in the case of the target vector,
 * a point along the direction of sight is formed by adding the target vector
 * to the eye point. In other words, gluLookAt() would be called as follows:
 *
 *   gluLookAt(e0, e1, e2, e0 + t0, e1 + t1, e2 + t2, u0, u1, u2);
 */
class CartesianKeyFrame
{
public:

    CartesianKeyFrame() {}
    CartesianKeyFrame(const CartesianKeyFrame &src);
    CartesianKeyFrame(
    double ex, double ey, double ez,
    double tx, double ty, double tz,
    double ux, double uy, double uz);
    virtual ~CartesianKeyFrame() {}

    mgp::math::_3DPoint* getEye() { return &eye_; }
    mgp::math::_3DPoint* getTgt() { return &tgt_; }
    mgp::math::_3DPoint* getUp()  {  return &up_; }
    void setEye(double x, double y, double z) { eye_.setPoint(x, y, z); }
    void setTgt(double x, double y, double z) { tgt_.setPoint(x, y, z); }
    void setUp(double x, double y, double z)  {  up_.setPoint(x, y, z); }
    void setEye(mgp::math::_3DPoint* p) { setEye(p->x(), p->y(), p->z()); }
    void setTgt(mgp::math::_3DPoint* p) { setTgt(p->x(), p->y(), p->z()); }
    void setUp( mgp::math::_3DPoint* p) { setUp( p->x(), p->y(), p->z()); }

private:
    mgp::math::_3DPoint eye_, tgt_, up_;
};

class QMouseEvent;
class QKeyEvent;
class QAction;

class GLWidget : public QGLWidget
{
Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);

    // Sets the focus (lookat) point of the camera.
    void setCameraFocus(double, double, double, bool = true);

    // Sets the normalized dolly value, clamping to range [0, 1].
    void setDolly(double dolly);    
    double dolly() const;

    void setCurrentFocusPos(double, double);
    mgp::Point currentFocusPos() const;

    void updateWIFilterPoint();
    void updateCurrCustomBasePolygonPoint();

private:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);

    // Computes the camera from the current focus, dolly, heading, and inclination.
    CartesianKeyFrame computeCamera();

    void computeRay(int x, int y, mgp::math::_4DPoint &eye, mgp::math::_4DPoint &ray);
    bool intersectsEarth(QMouseEvent *event, double &lon, double &lat);

    mgp::math::_3DPoint focus_;
    double dolly_, heading_, incl_;

    static const double minDolly_, maxDolly_;

    // current surface position
    double currLon_; // [-PI/2, PI/2]
    double currLat_; // [-PI, PI]

    // current focus position
    double focusLon_;
    double focusLat_;
    double focus_alt_;

    // current mouse position
    double mouseLon_;
    double mouseLat_;

    // whether the mouse currently intersects the earth
    bool mouseHitsEarth_;

    int dragBaseX_;
    int dragBaseY_;
    double dragBaseLon_;
    double dragBaseLat_;
    bool draggingWIFilter_;
    bool draggingOtherFilter_;
    bool draggingCustomBasePolygon_;
    bool draggingFocus_;

    QAction *addWIFilterPointAction_;
    QAction *removeWIFilterPointAction_;
    QAction *addCustomBasePolygonPointAction_;
    QAction *removeCustomBasePolygonPointAction_;

    float minBallSize_;
    float maxBallSize_;

    QList<mgp::FilterBase::Type> lonOrLatFilterTypes_;
    QList<mgp::FilterBase::Type> freeLineFilterTypes_;

    int currWIFilterPoint_; // index of WI filter point
    int currCustomBasePolygonPoint_; // index of current custom base polygon point

    double ballSize() const;

private slots:
    void addWIFilterPoint();
    void removeWIFilterPoint();
    void addCustomBasePolygonPoint();
    void removeCustomBasePolygonPoint();

signals:
    void focusPosChanged();
};

#endif // GLWIDGET_H
