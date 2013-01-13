#include <QtCore>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QVariantMap vmap;

    vmap.insert("a", 4711);
    vmap.insert("b", "hei");
    vmap.insert("c", "hallo");
    vmap.insert("d", false);
    vmap.insert("e", 3.14);

    // *** Basic access ***
    qDebug() << "vmap:" << vmap;
    qDebug() << "vmap.value(d):" << vmap.value("d");
    qDebug() << "vmap.value(d).toBool() (expect false):" << vmap.value("d").toBool();
    vmap.insert("d", true);
    qDebug() << "vmap.value(d).toBool() (excpect true):" << vmap.value("d").toBool();

    // *** Serialization **'
    // write to byte array
    QByteArray ba;
    QDataStream dstream(&ba, QIODevice::Append);
    dstream << vmap;
    // read back from byte array
    const QByteArray ba2 = ba; // copy ba to ba2 (equivalent to a network transfer!)
    QDataStream dstream2(ba2);
    QVariantMap vmap2;
    dstream2 >> vmap2;
    qDebug() << "\n";
    qDebug() << "vmap2:" << vmap2;
    qDebug() << "vmap2.value(d):" << vmap2.value("d");
    qDebug() << "vmap2.value(d).toBool() (excpect true):" << vmap2.value("d").toBool();

    return 0;
}
