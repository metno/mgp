#include <QtGui>
#include <QDebug>
#include "callgrindtester.h"

class TestWidget : public QWidget
{
  Q_OBJECT

public:
  TestWidget()
    : pos_(100, 100)
    , baseEvent_(QMouseEvent(QEvent::MouseMove, QPoint(), QPoint(), Qt::NoButton, Qt::MouseButtons(), Qt::KeyboardModifiers()))
  {
    resize(500, 500);
  }

public slots:
  void action(int step, int nSteps)
  {
    if (step == 1)
      qDebug() << "starting test";
    else if (step == nSteps)
      qDebug() << "test finished";

    // simulate circular mouse move
    const qreal frac = step / qreal(nSteps);
    const QPoint deltaPos(50 * cos(M_PI * 2 * frac), 50 * sin(M_PI * 2 * frac));
    QMouseEvent *event = new QMouseEvent(
          QEvent::MouseMove,
          baseEvent_.pos() + deltaPos,
          baseEvent_.globalPos() + deltaPos,
          baseEvent_.button(),
          baseEvent_.buttons(),
          baseEvent_.modifiers()
          );
    qApp->postEvent(this, event);
  }

private:
  CallgrindTester cgTester_;
  QPoint pos_;
  QMouseEvent baseEvent_;

  void paintEvent(QPaintEvent *)
  {
    QPainter painter(this);
    painter.setPen(Qt::blue);
    painter.setFont(QFont("Arial", 20));
    //painter.drawText(pos_, "Right-click anywhere to start test");
    painter.drawText(QRectF(pos_, QSizeF(200, 200)), Qt::TextWordWrap, "Right-click\nanywhere\nto start test");
  }

  void mousePressEvent(QMouseEvent *event)
  {
    pos_ = event->pos();
    update();

    if (event->button() == Qt::RightButton) {
      baseEvent_ = *event;
      cgTester_.start(300, 10, this, "action");
    }
  }

  void mouseMoveEvent(QMouseEvent *event)
  {
    pos_ = event->pos();
    update();
  }
};

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  TestWidget testWidget;
  testWidget.show();
  return app.exec();
}

#include "main.moc"
