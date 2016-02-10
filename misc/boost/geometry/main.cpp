//#include "mgp.h"
#include <iostream>
#include <boost/geometry.hpp>
#include <boost/version.hpp>
#include <QApplication>

using namespace std;
namespace bg = boost::geometry;

void testTransform(int argc, char *argv[])
{
    if (argc != 3) {
        cerr << "usage: " << argv[0] << " <lon degrees> <lat degrees>\n";
        qApp->exit(1);
        return;
    }

    bool ok = false;
    const double lon = QString(argv[1]).toInt(&ok);
    if (!ok) {
        cerr << "failed to extract longitude from >" << argv[1] << "<\n";
        qApp->exit(1);
        return;
    }
    const double lat = QString(argv[2]).toInt(&ok);
    if (!ok) {
        cerr << "failed to extract latitude from >" << argv[2] << "<\n";
        qApp->exit(1);
        return;
    }

    bg::model::point<long double, 2, bg::cs::spherical<bg::degree> > deg(lon, lat);

    // Transform from degree to radian. Default strategy is automatically selected,
    // it will convert from degree to radian
    bg::model::point<long double, 2, bg::cs::spherical<bg::radian> > rad;
    bg::transform(deg, rad);

    // Transform from degree (lon-lat) to 3D (x,y,z). Default strategy is automatically selected,
    // it will consider points on a unit sphere
    bg::model::point<long double, 3, bg::cs::cartesian> car;
    bg::transform(deg, car);

    cout
        << "  (lon deg, lat deg): " << bg::dsv(deg) << endl
        << "  (lon rad, lat rad): " << bg::dsv(rad) << endl
        << "(cartesian; x, y, z): " << bg::dsv(car) << endl;
}


void testPointInSphericalPolygon(int argc, char *argv[])
{
    typedef bg::model::point<double, 2, bg::cs::spherical<bg::radian> > point_t;
    typedef bg::model::polygon<point_t> polygon_t;

    polygon_t polygon;
    bg::append(polygon.outer(), point_t(0, 0));
    bg::append(polygon.outer(), point_t(0.2, 0.2));
    bg::append(polygon.outer(), point_t(0, 0.8));

    const point_t point(0.1, 0.1);
    //std::cerr << bg::dsv(point) << std::endl;
    //std::cerr << bg::get<0>(point) << ", " << bg::get<1>(point) << std::endl;
    //std::cerr << point.get<0>() << ", " << point.get<1>() << std::endl;

    const bool inside = bg::within(point, polygon); // APPARENTLY NOT SUPPORTED FOR SPHERICAL COORDINATES!
//    const bool inside = bg::within(point, polygon, bg::strategy::within::winding<point_t, point_t, void>());

    cout << "point is inside polygon: " << inside << endl;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

//    cout << "boost major version: " << (BOOST_VERSION / 100000) << endl;
//    cout << "boost minor version: " << (BOOST_VERSION / 100 % 1000) << endl;
//    cout << "boost patch level: " << (BOOST_VERSION % 100) << endl;


//    testTransform(argc, argv);
    testPointInSphericalPolygon(argc, argv);

    return 0;
}
