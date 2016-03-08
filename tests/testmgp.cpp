#include "mgp.h"
#include <QtTest/QtTest>

class TestMgp: public QObject
{
    Q_OBJECT
private slots:
    void test1();
};

void TestMgp::test1()
{
    mgp::Filters filters(new QList<mgp::Filter>);
    filters->append(mgp::Filter(new mgp::EOfFilter));
    filters->append(mgp::Filter(new mgp::SOfFilter));
    QCOMPARE(filters->size(), 2);

    mgp::Polygons inPolys(new QVector<mgp::Polygon>());
    mgp::Polygons outPolys = mgp::applyFilters(inPolys, filters);
    QVERIFY(outPolys->isEmpty());
}

QTEST_MAIN(TestMgp)
#include "testmgp.moc"
