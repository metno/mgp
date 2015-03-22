#include "laneheaderscene.h"
#include "laneheaderview.h"
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

    laneHeaderScene_ = new LaneHeaderScene(0, 0, 100, 2000);
    LaneHeaderView *laneHeaderView = new LaneHeaderView(laneHeaderScene_);
    splitter->addWidget(laneHeaderView);

    laneScene_ = new LaneScene(laneHeaderScene_, 2000);
    LaneView *laneView = new LaneView(laneScene_);
    splitter->addWidget(laneView);

    splitter->setSizes(QList<int>() << 100 << 400);

    mainLayout->addWidget(splitter);

    mainLayout->addWidget(new QPushButton("Test"));

    setLayout(mainLayout);
    setWindowTitle("Timeline");

    connect(TaskManager::instance(), SIGNAL(updated()), SLOT(refresh()));
    connect(laneHeaderView, SIGNAL(resized()), SLOT(refresh()));

    connect(laneView->verticalScrollBar(), SIGNAL(valueChanged(int)), laneHeaderView->verticalScrollBar(), SLOT(setValue(int)));
    connect(laneHeaderView->verticalScrollBar(), SIGNAL(valueChanged(int)), laneView->verticalScrollBar(), SLOT(setValue(int)));

    resize(1500, 500);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
}

void MainWindow::refresh()
{
    laneHeaderScene_->refresh();
    laneScene_->refresh();
}
