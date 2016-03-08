#include "mgp.h"
#include <QtTest/QtTest>

class TestMgp: public QObject
{
    Q_OBJECT
private slots:
    void testEmpty();
};

void TestMgp::testEmpty()
{
    mgp::Filters filters(new QList<mgp::Filter>);
    filters->append(mgp::Filter(new mgp::EOfFilter));
    filters->append(mgp::Filter(new mgp::SOfFilter));

    // single input polygon case 1
    {
        mgp::Polygon inPoly(new QVector<mgp::Point>());
        mgp::Polygons outPolys = mgp::applyFilters(inPoly, filters);
        QVERIFY(outPolys);
        QVERIFY(outPolys->isEmpty());
    }

    // single input polygon case 2
    {
        mgp::Polygon inPoly;
        mgp::Polygons outPolys = mgp::applyFilters(inPoly, filters);
        QVERIFY(outPolys);
        QVERIFY(outPolys->isEmpty());
    }

    // multiple input polygons case 1
    {
        mgp::Polygons inPolys(new QVector<mgp::Polygon>());
        mgp::Polygons outPolys = mgp::applyFilters(inPolys, filters);
        QVERIFY(outPolys);
        QVERIFY(outPolys->isEmpty());
    }

    // multiple input polygons case 2
    {
        mgp::Polygons inPolys;
        mgp::Polygons outPolys = mgp::applyFilters(inPolys, filters);
        QVERIFY(outPolys);
        QVERIFY(outPolys->isEmpty());
    }
}

QTEST_MAIN(TestMgp)
#include "testmgp.moc"
