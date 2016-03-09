#include "mgp.h"
#include <QtTest/QtTest>

class TestMgp: public QObject
{
    Q_OBJECT

private:
    /// Returns true iff two polygons are considered equal.
    static bool equal(const mgp::Polygon &poly1, const mgp::Polygon &poly2);

    /// Returns true iff a polygon is considered empty.
    static bool empty(const mgp::Polygon &poly);

    /// Returns true iff a set of polygons is considered empty.
    static bool empty(const mgp::Polygons &polys);

    /// Returns true iff two sets of polygons are considered equal.
    static bool equal(const mgp::Polygons &polys1, const mgp::Polygons &polys2);

private slots:
    void applyFiltersWithEmptyInput_data();
    void applyFiltersWithEmptyInput();

    void applyFiltersOverloadWithEmptyInput_data();
    void applyFiltersOverloadWithEmptyInput();

    void applyFiltersOverload_data();
    void applyFiltersOverload();
};
