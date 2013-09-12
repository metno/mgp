#include <QtGui>

static QString formatPoints(const QList<QPointF> &points)
{
  QString s;
  foreach (QPointF point, points) {
    const double lat = point.x();
    const double lon = point.y();
    s.append(QString("N%1 E%2, ").arg(lat).arg(lon));
  }
  return s;
}

class SpecialLineEdit : public QLineEdit
{
  Q_OBJECT
public:
  SpecialLineEdit(QWidget * parent = 0) : QLineEdit(parent) {}
  SpecialLineEdit(const QString & contents, QWidget * parent = 0) : QLineEdit(contents, parent) {}
private:
  void keyPressEvent(QKeyEvent *event) // reimplemented
  {
    if (event->matches(QKeySequence::Paste))
      specialPaste();
    else
      QLineEdit::keyPressEvent(event);
  }

  void contextMenuEvent(QContextMenuEvent *event) // reimplemented
  {
      QMenu *menu = createStandardContextMenu();
      foreach (QAction *action, menu->actions()) {
        if (action->text().toLower().contains("paste")) {
//          qDebug() << "paste action:" << action << "; text:" << action->text();
          action->dumpObjectInfo();
          disconnect(action, 0, 0, 0);
          connect(action, SIGNAL(triggered()), this, SLOT(specialPaste()));
        }
      }

      menu->exec(event->globalPos());
      delete menu;
  }

private slots:
  void specialPaste()
  {
    const QMimeData *data = QApplication::clipboard()->mimeData();
    if (data->hasFormat("application/x-diana-object")) {
      QByteArray bytes = data->data("application/x-diana-object");
      QDataStream stream(&bytes, QIODevice::ReadOnly);
      QVariantMap mainVMap;
      stream >> mainVMap;
      const QVariantList weatherAreas = mainVMap.value("weatherAreas").toList();
      if (weatherAreas.size() > 0) {
        const QVariant weatherArea = weatherAreas.first(); // consider only the first one
        const QVariantMap properties = weatherArea.toMap();
        const QVariantList vpoints = properties.value("points").toList();
        QList<QPointF> points;
        foreach (QVariant vpoint, vpoints)
          points.append(vpoint.toPointF());
        insert(formatPoints(points));
      }
    } else {
      paste(); // do standard pasting
    }
  }
};


class Window : public QWidget
{
public:
  Window()
  {
    QGridLayout *layout = new QGridLayout();

    layout->addWidget(new QLabel("normal QLineEdit:"), 0, 0);
    layout->addWidget(new QLineEdit, 0, 1);

    layout->addWidget(new QLabel("special QLineEdit:"), 1, 0);
    layout->addWidget(new SpecialLineEdit, 1, 1);

    setLayout(layout);
  }
};

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  Window window;
  window.show();
  return app.exec();
}

#include "main.moc"
