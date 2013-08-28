#include <QtGui>
#include <QtOpenGL>

#include "edititemmanager.h"
#include "rectangle.h"
#include "multiline.h"

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

    void mouseDoubleClickEvent(QMouseEvent *event)
    {
        QMouseEvent fe = flippedYPos(event);
        emit mouseDoubleClicked(&fe);
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
    void mouseDoubleClicked(QMouseEvent *);
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
        QHBoxLayout *topLayout = new QHBoxLayout;

        QVBoxLayout *leftLayout = new QVBoxLayout;

        QPushButton *button1 = new QPushButton("create rectangle\n(automatic placement)");
        button1->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(button1, SIGNAL(clicked()), this, SLOT(addRectangle()));
        leftLayout->addWidget(button1);

        QPushButton *button1_2 = new QPushButton("create rectangle\n(manual placement)");
        button1_2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(button1_2, SIGNAL(clicked()), this, SLOT(addRectangleManually()));
        leftLayout->addWidget(button1_2);

        QPushButton *button1_3 = new QPushButton("create rectangle\n(manual placement w/resize)");
        button1_3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(button1_3, SIGNAL(clicked()), this, SLOT(addRectangleManuallyWithResize()));
        leftLayout->addWidget(button1_3);

        QPushButton *button2 = new QPushButton("create multiline\n(manual placement)");
        button2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(button2, SIGNAL(clicked()), this, SLOT(addMultiLineManually()));
        leftLayout->addWidget(button2);

        QPushButton *button3 = new QPushButton("create polygon");
        button3->setEnabled(false); // for now
        button3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        leftLayout->addWidget(button3);

        QPushButton *button4 = new QPushButton("create polygon cutter");
        button4->setEnabled(false); // for now
        button4->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        leftLayout->addWidget(button4);

        QLabel *cblabel = new QLabel("CLIPBOARD DEMO");
        cblabel->setStyleSheet("QLabel { background-color : yellow }");
        cblabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        leftLayout->addWidget(cblabel);

        cbTextEdit_ = new QTextEdit;
        cbTextEdit_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        leftLayout->addWidget(cbTextEdit_);

        QPushButton *button5 = new QPushButton("copy to clipboard");
        button5->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(button5, SIGNAL(clicked()), this, SLOT(copyToClipboard()));
        leftLayout->addWidget(button5);

        QPushButton *button6 = new QPushButton("paste from clipboard");
        button6->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(button6, SIGNAL(clicked()), this, SLOT(pasteFromClipboard()));
        leftLayout->addWidget(button6);

        leftLayout->addStretch();
        topLayout->addLayout(leftLayout);

        QVBoxLayout *rightLayout = new QVBoxLayout;
        canvas_ = new Canvas;
        canvas_->show();
        rightLayout->addWidget(canvas_);
        topLayout->addLayout(rightLayout);

        //---------------------------------------------------------------------------

        QHBoxLayout *layout2 = new QHBoxLayout;

        undoButton_ = new QPushButton("undo");
        undoButton_->setEnabled(false);
        undoButton_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(undoButton_, SIGNAL(clicked()), editItemMgr_, SLOT(undo()));
        layout2->addWidget(undoButton_);

        redoButton_ = new QPushButton("redo");
        redoButton_->setEnabled(false);
        redoButton_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(redoButton_, SIGNAL(clicked()), editItemMgr_, SLOT(redo()));
        layout2->addWidget(redoButton_);

        layout2->addStretch();

        QUndoView *undoView = new QUndoView(editItemMgr_->undoStack());

        QVBoxLayout *bottomLayout = new QVBoxLayout;
        bottomLayout->addLayout(layout2);
        bottomLayout->addWidget(undoView);

        //---------------------------------------------------------------------------

        QVBoxLayout *mainLayout = new QVBoxLayout;
        //mainLayout->setContentsMargins(0, 0, 0, 0);

        QSplitter *splitter = new QSplitter(Qt::Vertical);

        QWidget *topWidget = new QWidget;
        topWidget->setLayout(topLayout);
        splitter->addWidget(topWidget);

        QWidget *bottomWidget = new QWidget;
        bottomWidget->setLayout(bottomLayout);
        splitter->addWidget(bottomWidget);

        splitter->setSizes(QList<int>() << 600 << 300);

        mainLayout->addWidget(splitter);
        setLayout(mainLayout);

        connect(canvas_, SIGNAL(mousePressed(QMouseEvent *)), editItemMgr, SLOT(mousePress(QMouseEvent *)));
        connect(canvas_, SIGNAL(mouseReleased(QMouseEvent *)), editItemMgr, SLOT(mouseRelease(QMouseEvent *)));
        connect(canvas_, SIGNAL(mouseMoved(QMouseEvent *)), editItemMgr, SLOT(mouseMove(QMouseEvent *)));
        connect(canvas_, SIGNAL(mouseDoubleClicked(QMouseEvent *)), editItemMgr, SLOT(mouseDoubleClick(QMouseEvent *)));
        connect(canvas_, SIGNAL(keyPressed(QKeyEvent *)), editItemMgr, SLOT(keyPress(QKeyEvent *)));
        connect(canvas_, SIGNAL(keyReleased(QKeyEvent *)), editItemMgr, SLOT(keyRelease(QKeyEvent *)));
        connect(canvas_, SIGNAL(paint()), editItemMgr, SLOT(draw()));
        connect(editItemMgr, SIGNAL(paintDone()), canvas_, SLOT(doSwapBuffers()));
        connect(editItemMgr, SIGNAL(repaintNeeded()), canvas_, SLOT(doRepaint()));
        connect(editItemMgr, SIGNAL(canUndoChanged(bool)), this, SLOT(handleCanUndoChanged(bool)));
        connect(editItemMgr, SIGNAL(canRedoChanged(bool)), this, SLOT(handleCanRedoChanged(bool)));
        connect(editItemMgr, SIGNAL(incompleteEditing(bool)), this, SLOT(handleIncompleteEditing(bool)));
    }

private:
    Canvas *canvas_;
    EditItemManager *editItemMgr_;
    QPushButton *undoButton_;
    QPushButton *redoButton_;
    QTextEdit *cbTextEdit_;

private slots:
    void addRectangle()
    {
        editItemMgr_->addItem(new EditItem_Rectangle::Rectangle);
        editItemMgr_->repaint();
    }

    void addRectangleManually()
    {
        editItemMgr_->addItem(new EditItem_Rectangle::Rectangle, true);
        editItemMgr_->repaint();
    }

    void addRectangleManuallyWithResize()
    {
        editItemMgr_->addItem(new EditItem_Rectangle::Rectangle(EditItem_Rectangle::Rectangle::Resize), true);
        editItemMgr_->repaint();
    }

    void addMultiLineManually()
    {
        editItemMgr_->addItem(new EditItem_MultiLine::MultiLine, true);
        editItemMgr_->repaint();
    }

    void copyToClipboard()
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(cbTextEdit_->toPlainText());
    }

    void pasteFromClipboard()
    {
        const QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();

        if (mimeData->hasImage()) {
            qDebug() << "mimeData->hasImage() ...";
            const QPixmap pixmap = qvariant_cast<QPixmap>(mimeData->imageData());
        } else if (mimeData->hasHtml()) {
            qDebug() << "mimeData->hasHtml() ...";
            const QString html = mimeData->html();
            qDebug() << "   html:" << html;
        } else if (mimeData->hasText()) {
            cbTextEdit_->setPlainText(mimeData->text());
        } else {
            qDebug() << "supported formats:" << mimeData->formats();
        }
    }
    void handleCanUndoChanged(bool canUndo)
    {
        undoButton_->setEnabled(canUndo);
    }

    void handleCanRedoChanged(bool canRedo)
    {
        redoButton_->setEnabled(canRedo);
    }

    void handleIncompleteEditing(bool incomplete)
    {
        if (incomplete) {
            setCursor(Qt::CrossCursor);
            canvas_->setFocus();
        } else {
            unsetCursor();
        }
    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window window(new EditItemManager);
    window.resize(800, 900);
    window.show();
    return app.exec();
}

#include "main.moc"
