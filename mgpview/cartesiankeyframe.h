#ifndef CARTESIANKEYFRAME_H
#define CARTESIANKEYFRAME_H

#include "util3d.h"

/**
 * This class represents a cartesian key frame: An eye point, a target vector,
 * and an up vector. The target- and up vectors are both normalized (having
 * length 1) and relative in the sense that, in the case of the target vector,
 * a point along the direction of sight is formed by adding the target vector
 * to the eye point. In other words, gluLookAt() would be called as follows:
 *
 *   gluLookAt(e0, e1, e2, e0 + t0, e1 + t1, e2 + t2, u0, u1, u2);
 *
 * \ingroup earthspheresequence
 *
 * @author Jo Asplin
 * @version $Revision: 1.3 $
 * @since 3.0
 */
class CartesianKeyFrame
{
public:

    CartesianKeyFrame();
    CartesianKeyFrame(
	double ex, double ey, double ez,
	double tx, double ty, double tz,
	double ux, double uy, double uz);
    CartesianKeyFrame(const CartesianKeyFrame& src);
    virtual ~CartesianKeyFrame();

    _3DPoint* getEye() {return &eye_;}
    _3DPoint* getTgt() {return &tgt_;}
    _3DPoint* getUp()  {return &up_;}
    void setEye(double x, double y, double z) {eye_.setPoint(x, y, z);}
    void setTgt(double x, double y, double z) {tgt_.setPoint(x, y, z);}
    void setUp(double x, double y, double z)  { up_.setPoint(x, y, z);}
    void setEye(_3DPoint* p) {setEye(p->x(), p->y(), p->z());}
    void setTgt(_3DPoint* p) {setTgt(p->x(), p->y(), p->z());}
    void setUp( _3DPoint* p) {setUp( p->x(), p->y(), p->z());}

private:

    _3DPoint eye_, tgt_, up_;
};

#endif // CARTESIANKEYFRAME_H
