#include "leftheaderscene.h"
#include "leftheaderview.h"
#include "lanescene.h"
#include "laneview.h"
#include "mainwindow.h"
#include "taskmanager.h"
#include "common.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QKeyEvent>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    QSplitter *splitter = new QSplitter;

    leftHeaderScene_ = new LeftHeaderScene(0, 0, 100, 2000);
    LeftHeaderView *leftHeaderView = new LeftHeaderView(leftHeaderScene_);
    splitter->addWidget(leftHeaderView);

    laneScene_ = new LaneScene(leftHeaderScene_, 2000);
    LaneView *laneView = new LaneView(laneScene_);
    splitter->addWidget(laneView);

    splitter->setSizes(QList<int>() << 100 << 400);

    mainLayout->addWidget(splitter);

    mainLayout->addWidget(new QPushButton("Test"));

    setLayout(mainLayout);
    setWindowTitle("Timeline");

    connect(TaskManager::instance(), SIGNAL(updated()), SLOT(refresh()));
    connect(leftHeaderView, SIGNAL(resized()), SLOT(refresh()));

    connect(laneView->verticalScrollBar(), SIGNAL(valueChanged(int)), leftHeaderView->verticalScrollBar(), SLOT(setValue(int)));
    connect(leftHeaderView->verticalScrollBar(), SIGNAL(valueChanged(int)), laneView->verticalScrollBar(), SLOT(setValue(int)));

    resize(1500, 500);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Escape) || ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Q)))
        close();
}

void MainWindow::refresh()
{
    leftHeaderScene_->refresh();
    laneScene_->refresh();
}
