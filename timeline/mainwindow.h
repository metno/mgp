#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

class Scene;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget * = 0);
private:
    Scene *scene_;
};

#endif // MAINWINDOW_H
