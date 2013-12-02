#include <QtGui>
#include <QDebug>
#include "callgrindtester.h"

class TestObject : public QObject
{
  Q_OBJECT
public slots:
  void startTest()
  {
    cgTester_.start(10, 500, this, "action");
  }

  void action(int step, int nSteps)
  {
    qDebug() << "action:" << step << ":" << nSteps;
  }

private:
  CallgrindTester cgTester_;
};


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QPushButton button("start test");
  TestObject testObject;
  QObject::connect(&button, SIGNAL(clicked()), &testObject, SLOT(startTest()));
  button.show();
  return app.exec();
}

#include "main.moc"
