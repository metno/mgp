#ifndef COMMON_H
#define COMMON_H

#include <QDebug>


// --- BEGIN remove when libmgp is complete ------
#include <QSharedPointer>
#include <QVector>
#include <QPair>

typedef QSharedPointer<QVector<QPair<double, double> > > PointVector;
typedef QSharedPointer<QVector<PointVector> > PointVectors;

#define USEMGP 1 // ### helps in refactoring into libmgp

// --- END remove when libmgp is complete ------


#endif // COMMON_H
