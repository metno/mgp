#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QString>

class GLWidget;
class QSlider;
class QWheelEvent;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    static void init(const QString &);
    static MainWindow &instance();
    void handleKeyPressEvent(QKeyEvent *);
    GLWidget *glWidget() const;

private:
    static QString initExpr_;
    static bool isInit_;
    MainWindow();
    GLWidget *glw_;
    QSlider *zoomSlider_;
    QSlider *lonSlider_;
    QSlider *latSlider_;

    virtual void keyPressEvent(QKeyEvent *);
    virtual void wheelEvent(QWheelEvent *);

    void syncZoomSlider();
    void syncLonSlider();
    void syncLatSlider();

private slots:
    void handleZoomValueChanged(int);
    void handleLonValueChanged(int);
    void handleLatValueChanged(int);
    void handleFocusPosChanged();
    void openControlPanel();
};

#endif // MAINWINDOW_H
