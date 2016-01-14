#include "cartesiankeyframe.h"

CartesianKeyFrame::CartesianKeyFrame()
{
}


CartesianKeyFrame::CartesianKeyFrame(
    double ex, double ey, double ez,
    double tx, double ty, double tz,
    double ux, double uy, double uz)
{
    setEye(ex, ey, ez);
    setTgt(tx, ty, tz);
    setUp (ux, uy, uz);
}


CartesianKeyFrame::CartesianKeyFrame(const CartesianKeyFrame& src)
    : eye_(src.eye_)
    , tgt_(src.tgt_)
    , up_(src.up_)
{
}


CartesianKeyFrame::~CartesianKeyFrame()
{
}
