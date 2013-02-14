#ifndef QCGLOBAL_H
#define QCGLOBAL_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QtGlobal>

// Global functions.

namespace qclib {

QMap<QString, QString> getOptions(const QStringList &);
QMap<QString, QString> getOptions(int argc, char *argv[]);
bool localServerFileExists(QString *, qint64 * = 0);

} // namespace qclib

#endif // QCGLOBAL_H
