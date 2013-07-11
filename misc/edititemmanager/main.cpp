#include <QtGui>
#include <QtOpenGL>

#include "edititemmanager.h"
#include "edititemx.h"

class Canvas : public QGLWidget
{
    Q_OBJECT
public:
    Canvas(QWidget *parent = 0)
        : QGLWidget(parent)
        , focus_(false)
    {
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
    }

private:
    void initializeGL()
    {
        glEnable(GL_DEPTH_TEST);
    }

    void resizeGL(int w, int h)
    {
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, 0, h, -2, 2);
     }

    void paintGL()
    {
        qglClearColor(QColor(204, 204, 204));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        emit paint();
    }

    QMouseEvent flippedYPos(QMouseEvent *event) const
    {
        return QMouseEvent(
            event->type(),
            QPoint(event->pos().x(), height() - event->pos().y()),
            event->globalPos(),
            event->button(),
            event->buttons(),
            event->modifiers()
            );
    }

    void mousePressEvent(QMouseEvent *event)
    {
        QMouseEvent fe = flippedYPos(event);
        emit mousePressed(&fe);
    }
    void mouseReleaseEvent(QMouseEvent *event)
    {
        QMouseEvent fe = flippedYPos(event);
        emit mouseReleased(&fe);
    }
    void mouseMoveEvent(QMouseEvent *event)
    {
        QMouseEvent fe = flippedYPos(event);
        emit mouseMoved(&fe);
    }
    void keyPressEvent(QKeyEvent *event) { emit keyPressed(event); }
    void keyReleaseEvent(QKeyEvent *event) { emit keyReleased(event); }

    void focusInEvent(QFocusEvent *)
    {
        focus_ = true;
        update();
    }

    void focusOutEvent(QFocusEvent *)
    {
        focus_ = false;
        update();
    }

    bool focus_;

signals:
    void mousePressed(QMouseEvent *);
    void mouseReleased(QMouseEvent *);
    void mouseMoved(QMouseEvent *);
    void keyPressed(QKeyEvent *);
    void keyReleased(QKeyEvent *);
    void paint();

public slots:
    void doSwapBuffers()
    {
        if (focus_) {
            glColor3ub(0, 128, 0);
            renderText(2, height() - 2, "keyboard focus");
        } else {
            glColor3ub(255, 0, 0);
            renderText(2, height() - 2, "no keyboard focus");
        }

        swapBuffers();
    }

    void doRepaint() { update(); }
};


class Window : public QWidget
{
    Q_OBJECT
public:
    Window(EditItemManager *editItemMgr)
        : editItemMgr_(editItemMgr)
    {
        QHBoxLayout *layout = new QHBoxLayout;

        QVBoxLayout *leftLayout = new QVBoxLayout;

        QPushButton *button1 = new QPushButton("create rectangle");
        button1->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(button1, SIGNAL(clicked()), this, SLOT(addItemType1()));
        leftLayout->addWidget(button1);

        QPushButton *button2 = new QPushButton("create polygon");
        button2->setEnabled(false); // for now
        button2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        leftLayout->addWidget(button2);

        QPushButton *button3 = new QPushButton("create polygon cutter");
        button3->setEnabled(false); // for now
        button3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        leftLayout->addWidget(button3);

        undoButton_ = new QPushButton("undo");
        undoButton_->setEnabled(false);
        undoButton_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(undoButton_, SIGNAL(clicked()), editItemMgr_, SLOT(undo()));
        leftLayout->addWidget(undoButton_);

        redoButton_ = new QPushButton("redo");
        redoButton_->setEnabled(false);
        redoButton_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(redoButton_, SIGNAL(clicked()), editItemMgr_, SLOT(redo()));
        leftLayout->addWidget(redoButton_);

        leftLayout->addStretch();
        layout->addLayout(leftLayout);

        QVBoxLayout *rightLayout = new QVBoxLayout;
        Canvas *canvas = new Canvas;
        canvas->show();
        rightLayout->addWidget(canvas);
        layout->addLayout(rightLayout);

        setLayout(layout);

        connect(canvas, SIGNAL(mousePressed(QMouseEvent *)), editItemMgr, SLOT(mousePress(QMouseEvent *)));
        connect(canvas, SIGNAL(mouseReleased(QMouseEvent *)), editItemMgr, SLOT(mouseRelease(QMouseEvent *)));
        connect(canvas, SIGNAL(mouseMoved(QMouseEvent *)), editItemMgr, SLOT(mouseMove(QMouseEvent *)));
        connect(canvas, SIGNAL(keyPressed(QKeyEvent *)), editItemMgr, SLOT(keyPress(QKeyEvent *)));
        connect(canvas, SIGNAL(keyReleased(QKeyEvent *)), editItemMgr, SLOT(keyRelease(QKeyEvent *)));
        connect(canvas, SIGNAL(paint()), editItemMgr, SLOT(draw()));
        connect(editItemMgr, SIGNAL(paintDone()), canvas, SLOT(doSwapBuffers()));
        connect(editItemMgr, SIGNAL(repaintNeeded()), canvas, SLOT(doRepaint()));
        connect(editItemMgr, SIGNAL(canUndoChanged(bool)), this, SLOT(handleCanUndoChanged(bool)));
        connect(editItemMgr, SIGNAL(canRedoChanged(bool)), this, SLOT(handleCanRedoChanged(bool)));
    }

private:
    EditItemManager *editItemMgr_;
    QPushButton *undoButton_;
    QPushButton *redoButton_;

private slots:
    void addItemType1()
    {
        editItemMgr_->addItem(new EditItemX);
        editItemMgr_->repaint();
    }

    void handleCanUndoChanged(bool canUndo)
    {
        undoButton_->setEnabled(canUndo);
    }

    void handleCanRedoChanged(bool canRedo)
    {
        redoButton_->setEnabled(canRedo);
    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window window(new EditItemManager);
    window.resize(800, 600);
    window.show();
    return app.exec();
}

#include "main.moc"
