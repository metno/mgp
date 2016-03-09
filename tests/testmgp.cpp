#include "mgp.h"
#include "mgpmath.h"
#include <QtTest/QtTest>

Q_DECLARE_METATYPE(mgp::Point)
Q_DECLARE_METATYPE(mgp::Polygon)
Q_DECLARE_METATYPE(mgp::Polygons)
Q_DECLARE_METATYPE(mgp::Filters)

class TestMgp: public QObject
{
    Q_OBJECT

private slots:
    void applyFiltersWithEmptyInput_data();
    void applyFiltersWithEmptyInput();

    void applyFiltersOverloadWithEmptyInput_data();
    void applyFiltersOverloadWithEmptyInput();

    void applyFiltersOverload_data();
    void applyFiltersOverload();
};

// Returns true iff two polygons are considered equal.
static bool equal(const mgp::Polygon &poly1, const mgp::Polygon &poly2)
{
    const bool isEmpty1 = (!poly1) || poly1->isEmpty();
    const bool isEmpty2 = (!poly2) || poly2->isEmpty();

    if (isEmpty1 && isEmpty2)
        return true; // both polygons are empty

    if ((isEmpty1 && (!isEmpty2)) || ((!isEmpty1) && isEmpty2))
        return false; // exactly one polygon is empty

    if (poly1->size() != poly2->size())
        return false; // polygons have different sizes

    return *poly1 == *poly2;
}

// Returns true iff a polygon is considered empty.
static bool empty(const mgp::Polygon &poly)
{
    return (!poly) || poly->isEmpty();
}

// Returns true iff a set of polygons is considered empty.
static bool empty(const mgp::Polygons &polys)
{
    if ((!polys) || polys->isEmpty())
        return true;
    for (int i = 0; i < polys->size(); ++i)
        if (!empty(polys->at(i)))
            return false; // at least one polygon is non-empty
    return true;
}

// Returns true iff two sets of polygons are considered equal.
static bool equal(const mgp::Polygons &polys1, const mgp::Polygons &polys2)
{
    const bool isEmpty1 = empty(polys1);
    const bool isEmpty2 = empty(polys2);

    if (isEmpty1 && isEmpty2)
        return true; // both sets are empty

    if ((isEmpty1 && (!isEmpty2)) || ((!isEmpty1) && isEmpty2))
        return false; // exactly one set is empty

    if (polys1->size() != polys2->size())
        return false; // sets have different sizes

    for (int i = 0; i < polys1->size(); ++i)
        if (!equal(polys1->at(i), polys2->at(i)))
            return false; // ith polygon differs in the two sets

    return true; // no difference detected
}

void TestMgp::applyFiltersWithEmptyInput_data()
{
    QTest::addColumn<mgp::Polygons>("polygons");
    QTest::addColumn<mgp::Filters>("filters");
    QTest::addColumn<mgp::Polygons>("expectedResult");

    const mgp::Polygons emptyPolys1;
    const mgp::Polygons emptyPolys2(new QVector<mgp::Polygon>());
    mgp::Polygons emptyPolys3(new QVector<mgp::Polygon>());
    emptyPolys3->append(mgp::Polygon(new QVector<mgp::Point>()));
    emptyPolys3->append(mgp::Polygon(new QVector<mgp::Point>()));

    const mgp::Filters emptyFilters1;
    const mgp::Filters emptyFilters2(new QList<mgp::Filter>);

    mgp::Polygons nonEmptyPolys(new QVector<mgp::Polygon>());
    nonEmptyPolys->append(mgp::Polygon(new QVector<mgp::Point>()));
    nonEmptyPolys->last()->append(qMakePair(0.0, 0.0));
    nonEmptyPolys->last()->append(qMakePair(1.0, 0.0));
    nonEmptyPolys->last()->append(qMakePair(1.0, 1.0));
    nonEmptyPolys->append(mgp::Polygon(new QVector<mgp::Point>()));
    nonEmptyPolys->last()->append(qMakePair(0.5, 0.5));
    nonEmptyPolys->last()->append(qMakePair(1.5, 0.5));
    nonEmptyPolys->last()->append(qMakePair(1.5, 1.5));

    mgp::Filters nonEmptyFilters(new QList<mgp::Filter>);
    nonEmptyFilters->append(mgp::Filter(new mgp::EOfFilter));
    nonEmptyFilters->append(mgp::Filter(new mgp::SOfFilter));

    //-------------------------------------------------------------
    QTest::newRow("empty polygons 1, empty filters 1") << emptyPolys1 << emptyFilters1 << emptyPolys1;
    QTest::newRow("empty polygons 2, empty filters 1") << emptyPolys2 << emptyFilters1 << emptyPolys1;
    QTest::newRow("empty polygons 3, empty filters 1") << emptyPolys3 << emptyFilters1 << emptyPolys1;
    QTest::newRow("empty polygons 1, empty filters 2") << emptyPolys1 << emptyFilters2 << emptyPolys1;
    QTest::newRow("empty polygons 2, empty filters 2") << emptyPolys2 << emptyFilters2 << emptyPolys1;
    QTest::newRow("empty polygons 3, empty filters 2") << emptyPolys3 << emptyFilters2 << emptyPolys1;

    //-------------------------------------------------------------
    QTest::newRow("empty polygons 1, non-empty filters") << emptyPolys1 << nonEmptyFilters << emptyPolys1;
    QTest::newRow("empty polygons 2, non-empty filters") << emptyPolys2 << nonEmptyFilters << emptyPolys1;
    QTest::newRow("empty polygons 3, non-empty filters") << emptyPolys3 << nonEmptyFilters << emptyPolys1;

    //-------------------------------------------------------------
    QTest::newRow("non-empty polygons, empty filters 1") << nonEmptyPolys << emptyFilters1 << nonEmptyPolys;
    QTest::newRow("non-empty polygons, empty filters 2") << nonEmptyPolys << emptyFilters2 << nonEmptyPolys;
}

void TestMgp::applyFiltersWithEmptyInput()
{
    QFETCH(mgp::Polygons, polygons);
    QFETCH(mgp::Filters, filters);
    QFETCH(mgp::Polygons, expectedResult);

    const mgp::Polygons outPolys = mgp::applyFilters(polygons, filters);
    QVERIFY(equal(outPolys, expectedResult));
}

void TestMgp::applyFiltersOverloadWithEmptyInput_data()
{
    QTest::addColumn<mgp::Polygon>("polygon");
    QTest::addColumn<mgp::Filters>("filters");
    QTest::addColumn<mgp::Polygons>("expectedResult");

    const mgp::Polygon emptyPoly1;
    const mgp::Polygon emptyPoly2(new QVector<mgp::Point>());

    const mgp::Filters emptyFilters1;
    const mgp::Filters emptyFilters2(new QList<mgp::Filter>);

    mgp::Polygon nonEmptyPoly(mgp::Polygon(new QVector<mgp::Point>()));
    nonEmptyPoly->append(qMakePair(0.0, 0.0));
    nonEmptyPoly->append(qMakePair(1.0, 0.0));
    nonEmptyPoly->append(qMakePair(1.0, 1.0));
    mgp::Polygons nonEmptyPolys(new QVector<mgp::Polygon>());
    nonEmptyPolys->append(nonEmptyPoly);

    mgp::Filters nonEmptyFilters(new QList<mgp::Filter>);
    nonEmptyFilters->append(mgp::Filter(new mgp::EOfFilter));
    nonEmptyFilters->append(mgp::Filter(new mgp::SOfFilter));

    const mgp::Polygons emptyPolys;

    //-------------------------------------------------------------
    QTest::newRow("empty polygon 1, empty filters 1") << emptyPoly1 << emptyFilters1 << emptyPolys;
    QTest::newRow("empty polygon 2, empty filters 1") << emptyPoly2 << emptyFilters1 << emptyPolys;
    QTest::newRow("empty polygon 1, empty filters 2") << emptyPoly1 << emptyFilters2 << emptyPolys;
    QTest::newRow("empty polygon 2, empty filters 2") << emptyPoly2 << emptyFilters2 << emptyPolys;

    //-------------------------------------------------------------
    QTest::newRow("empty polygon 1, non-empty filters") << emptyPoly1 << nonEmptyFilters << emptyPolys;
    QTest::newRow("empty polygon 2, non-empty filters") << emptyPoly2 << nonEmptyFilters << emptyPolys;

    //-------------------------------------------------------------
    QTest::newRow("non-empty polygons, empty filters 1") << nonEmptyPoly << emptyFilters1 << nonEmptyPolys;
    QTest::newRow("non-empty polygons, empty filters 2") << nonEmptyPoly << emptyFilters2 << nonEmptyPolys;
}

void TestMgp::applyFiltersOverloadWithEmptyInput()
{
    QFETCH(mgp::Polygon, polygon);
    QFETCH(mgp::Filters, filters);
    QFETCH(mgp::Polygons, expectedResult);

    const mgp::Polygons outPolys = mgp::applyFilters(polygon, filters);
    QVERIFY(equal(outPolys, expectedResult));
}

void TestMgp::applyFiltersOverload_data()
{
    QTest::addColumn<mgp::Polygon>("polygon");
    QTest::addColumn<mgp::Filters>("filters");
    QTest::addColumn<mgp::Point>("point");
    QTest::addColumn<bool>("inside");

    mgp::Polygon polygon(new QVector<mgp::Point>());
    mgp::Filters filters(new QList<mgp::Filter>());

    //-------------------------------------------------------------
    polygon->clear();
    polygon->append(qMakePair(DEG2RAD(0), DEG2RAD(60)));
    polygon->append(qMakePair(DEG2RAD(10), DEG2RAD(60)));
    polygon->append(qMakePair(DEG2RAD(5), DEG2RAD(70)));
    filters->clear();
    filters->append(mgp::Filter(new mgp::SOfFilter(DEG2RAD(65))));
    QTest::newRow("S_OF inside") << polygon << filters << qMakePair(DEG2RAD(5), DEG2RAD(63)) << true;
    QTest::newRow("S_OF outside") << polygon << filters << qMakePair(DEG2RAD(5), DEG2RAD(67)) << false;
}

void TestMgp::applyFiltersOverload()
{
    QFETCH(mgp::Polygon, polygon);
    QFETCH(mgp::Filters, filters);
    QFETCH(mgp::Point, point);
    QFETCH(bool, inside);

    const mgp::Polygons outPolys = mgp::applyFilters(polygon, filters);
    int nInsides = 0;
    for (int i = 0; i < outPolys->size(); ++i)
        if (mgp::math::pointInPolygon(point, outPolys->at(i)))
            nInsides++;
    QVERIFY((inside && (nInsides > 0)) || ((!inside) && (nInsides == 0)));
}

QTEST_MAIN(TestMgp)
#include "testmgp.moc"
