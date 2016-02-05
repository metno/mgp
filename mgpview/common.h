#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include <QSharedPointer>
#include <QVector>
#include <QPair>

typedef QSharedPointer<QVector<QPair<double, double> > > PointVector;
typedef QSharedPointer<QVector<PointVector> > PointVectors;

#endif // COMMON_H
