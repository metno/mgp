#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "cartesiankeyframe.h"
#include <QGLWidget>
#include <QPair>
#include <QMouseEvent>

class QAction;

class GLWidget : public QGLWidget
{
Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);

    /** Sets the focus (lookat) point of the camera. */
    void setCameraFocus(double, double, double, bool = true);

    /** Sets the normalized dolly value, clamping to range [0, 1]. */
    void setDolly(double dolly);    
    double dolly() const;

    void setCurrentFocusPos(double, double);
    QPair<double, double> currentFocusPos() const;

    /** Sets the normalized heading value, clamping to range [0, 1]. */
    void setHeading(double heading);

    /** Sets the normalized inclination value, clamping to range [0, 1]. */
    void setInclination(double incl);

    void enableInsertion(bool on);

    void setCamKFSLaveMode(bool on) {camKfSlaveMode_ = on;}

    void setBallSizeFrac(float);
    float ballSizeFrac() const;

private:

    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);

    void enterEvent(QEvent *);
/*
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);
*/

    /** Computes the camera from the current focus, dolly, heading, and
     * inclination.
     */
    CartesianKeyFrame computeCamera();

    void computeRay(int x, int y, _4DPoint &eye, _4DPoint &ray);
    void computePixel(double wx, double wy, double wz, int &x, int &y);
    void drawLabel(const char s[], int x, int y, float r, float g, float b);
    bool intersectCartesianKeyFrame(
    _4DPoint &eye, _4DPoint &ray, CartesianKeyFrame &ckf);
//    bool intersectKeyFrameBase(
//    _4DPoint &eye, _4DPoint &ray, KeyFrame &kf);
    bool intersectInsertKeyFrame(_4DPoint &eye, _4DPoint &ray);
    bool intersectInsertKeyFrameBase(_4DPoint &eye, _4DPoint &ray);
    bool intersectKeyFrame(_4DPoint &eye, _4DPoint &ray, int &i);
    bool intersectCurrentKeyFrameBase(_4DPoint &eye, _4DPoint &ray);
    bool intersectEarth(
    _4DPoint &eye, _4DPoint &ray, double &wx, double &wy,
    double &wz);
    void insert(bool after);

    double min(double a, double b) {return a < b ? a : b;}
    double max(double a, double b) {return a > b ? a : b;}

//    void toggleVisMenuItem(int item);
//    void enableKeyFrameDependentActions(bool on);

    _3DPoint focus_;
    double dolly_, heading_, incl_;

    static const double minDolly_, maxDolly_;

    /** The camera used the last time the draw() method of the current
     * earth-sphere sequence was called.
     */
    CartesianKeyFrame lastCam_;

    bool camKfSlaveMode_;

    // current surface position
    double currLon_; // [-PI/2, PI/2]
    double currLat_; // [-PI, PI]

    int isctKf_, removeItem_, insertBeforeItem_, insertAfterItem_,
    explicitFocusItem_, gravityFocusItem_, currentKfFocusItem_,
    drawKfLabelsItem_, drawCameraItem_, drawFocusPointItem_,
    drawFocusPointLabelItem_;

    // current focus position
    double focusLon_;
    double focusLat_;
    double focus_alt_;

    // current mouse position
    double mouseLon_;
    double mouseLat_;

    int dragBaseX_;
    int dragBaseY_;
    double dragBaseFocusLon_;
    double dragBaseFocusLat_;
    bool dragging_;

    QAction *setCurrPosFromDialogAction_;
    QAction *setCurrPosToThisPosAction_;
    QAction *focusOnThisPosAction_;
    QAction *focusOnCurrPosAction_;

    float ballSize_;
    float minBallSize_;
    float maxBallSize_;

private slots:
    void drawCalled(QObject *ckf);
    void setCurrPosFromDialog();
    void setCurrPosToThisPos();
    void focusOnThisPos();
    void focusOnCurrPos();

//    void toggleLabels();
//    void toggleCameraIndicator();
//    void toggleFocusPoint();
//    void toggleFocusPointLabel();
//    void removeKeyFrame();
//    void insertAfterKeyFrame();
//    void insertBeforeKeyFrame();
//    void sizeChanged();

signals:
    void focusPosChanged();
};

#endif // GLWIDGET_H
