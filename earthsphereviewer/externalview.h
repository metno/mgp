#ifndef EXTERNALVIEW_H
#define EXTERNALVIEW_H

//#include <qgl.h>
//#include <qpopupmenu.h>
//#include <MeEarthSphereSequence.h>

#include "cartesiankeyframe.h"
#include <QGLWidget>


/**
 * This class represents the external view of the key frame sequence.
 *
 * <strong>MORE DETAILS TO COME!</strong>
 *
 * <strong>NOTICE:</strong> Since this class is used as a custom widget in
 * Qt Designer, it has to be outside the 'Me' namespace (more specifically,
 * the Qt User Interface Compiler (uic) generates code that refers to the
 * 'ExternalView' class name outside the 'Me' namespace).
 *
 * \ingroup earthspheresequence_gui
 *
 * @author Jo Asplin
 * @version $Id: ExternalView.h,v 1.18 2002/05/21 15:51:36 joa Exp $
 * @since 3.0
 */
class ExternalView : public QGLWidget
{
Q_OBJECT

public:
    ExternalView(QWidget *parent = 0);

    /** Sets the focus (lookat) point of the camera. */
    void setCameraFocus(double x, double y, double z, bool update_gl = true);

    /** Sets the normalized dolly value, clamping to range [0, 1]. */
    void setDolly(double dolly);

    /** Sets the normalized heading value, clamping to range [0, 1]. */
    void setHeading(double heading);

    /** Sets the normalized inclination value, clamping to range [0, 1]. */
    void setInclination(double incl);

    void enableInsertion(bool on);

    void setCamKFSLaveMode(bool on) {cam_kf_slave_mode_ = on;}

private:

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void enterEvent(QEvent*);
/*
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);
*/

    /** Computes the camera from the current focus, dolly, heading, and
     * inclination.
     */
    CartesianKeyFrame computeCamera();

    void computeRay(int x, int y, _4DPoint& eye, _4DPoint& ray);
    void computePixel(double wx, double wy, double wz, int& x, int& y);
    void drawLabel(const char s[], int x, int y, float r, float g, float b);
    bool intersectCartesianKeyFrame(
    _4DPoint& eye, _4DPoint& ray, CartesianKeyFrame& ckf);
//    bool intersectKeyFrameBase(
//    _4DPoint& eye, _4DPoint& ray, KeyFrame& kf);
    bool intersectInsertKeyFrame(_4DPoint& eye, _4DPoint& ray);
    bool intersectInsertKeyFrameBase(_4DPoint& eye, _4DPoint& ray);
    bool intersectKeyFrame(_4DPoint& eye, _4DPoint& ray, int& i);
    bool intersectCurrentKeyFrameBase(_4DPoint& eye, _4DPoint& ray);
    bool intersectEarth(
    _4DPoint& eye, _4DPoint& ray, double& wx, double& wy,
	double& wz);
    void insert(bool after);

    double min(double a, double b) {return a < b ? a : b;}
    double max(double a, double b) {return a > b ? a : b;}

//    void toggleVisMenuItem(int item);
//    void enableKeyFrameDependentActions(bool on);

    _3DPoint focus_;
    double dolly_, heading_, incl_;

    static const double min_dolly_, max_dolly_;

    /** The camera used the last time the draw() method of the current
     * earth-sphere sequence was called.
     */
    CartesianKeyFrame last_cam_;

    bool cam_kf_slave_mode_;

    bool curr_base_dragging_;
    double lat_, lon_;
    int isct_kf_, remove_item_, insert_before_item_, insert_after_item_,
	explicit_focus_item_, gravity_focus_item_, current_kf_focus_item_,
	draw_kf_labels_item_, draw_camera_item_, draw_focus_point_item_,
	draw_focus_point_label_item_;

    double focus_lat_, focus_lon_, focus_alt_;
    bool focus_lock_to_curr_;
    //QPopupMenu* global_menu_, * focus_menu_, * vis_menu_, * kf_menu_;

private slots:
    void drawCalled(QObject* ckf);
    void setFocusToSurfacePoint();
//    void toggleLabels();
//    void toggleCameraIndicator();
//    void toggleFocusPoint();
//    void toggleFocusPointLabel();
//    void removeKeyFrame();
//    void insertAfterKeyFrame();
//    void insertBeforeKeyFrame();
//    void sizeChanged();

signals:
    void updateCurrentLatLon();
};

#endif // EXTERNALVIEW_H
