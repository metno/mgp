#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "cartesiankeyframe.h"
#include "controlpanel.h"
#include <QGLWidget>
#include <QPair>
#include <QLineF>
#include <QVariant>
#include <QHash>

class QMouseEvent;
class QKeyEvent;
class QAction;

struct LonOrLatFilterInfo {
    Filter::Type type;
    bool isLon;
    LonOrLatFilterInfo(Filter::Type, bool);
};

struct FreeLineFilterInfo {
    Filter::Type type;
    FreeLineFilterInfo(Filter::Type);
};

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
    QPair<double, double> currentFocusPos() const;

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

    // Computes the camera from the current focus, dolly, heading, and inclination.
    CartesianKeyFrame computeCamera();

    void computeRay(int x, int y, _4DPoint &eye, _4DPoint &ray);
    bool intersectsEarth(QMouseEvent *event, double &lon, double &lat);

    _3DPoint focus_;
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
    bool draggingFilter_;
    bool draggingCustomBasePolygonPoint_;
    bool draggingFocus_;

    QAction *addCustomBasePolygonPointAction_;
    QAction *removeCustomBasePolygonPointAction_;

    float minBallSize_;
    float maxBallSize_;

    // ### THESE COULD BE JUST QLists !!! (the key doesn't matter)
    QHash<int, LonOrLatFilterInfo *> lonOrLatFilterInfos_;
    QHash<int, FreeLineFilterInfo *> freeLineFilterInfos_;

    int currCustomBasePolygonPoint_; // index of current custom base polygon point

    double ballSize() const;

private slots:
    void addCustomBasePolygonPoint();
    void removeCustomBasePolygonPoint();

signals:
    void focusPosChanged();
};

#endif // GLWIDGET_H
