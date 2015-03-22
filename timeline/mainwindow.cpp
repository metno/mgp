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


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    QSplitter *splitter = new QSplitter;

    LaneHeaderScene *laneHeaderScene = new LaneHeaderScene(0, 0, 100, 2000);
    connect(TaskManager::instance(), SIGNAL(updated()), laneHeaderScene, SLOT(refresh()));
    LaneHeaderView *laneHeaderView = new LaneHeaderView(laneHeaderScene);
    connect(laneHeaderView, SIGNAL(resized()), laneHeaderScene, SLOT(refresh()));
    splitter->addWidget(laneHeaderView);

    LaneScene *laneScene = new LaneScene(laneHeaderScene, 2000);
    connect(TaskManager::instance(), SIGNAL(updated()), laneScene, SLOT(refresh()));
    LaneView *laneView = new LaneView(laneScene);
    splitter->addWidget(laneView);

    splitter->setSizes(QList<int>() << 100 << 400);

    mainLayout->addWidget(splitter);

    mainLayout->addWidget(new QPushButton("Test"));

    setLayout(mainLayout);
    setWindowTitle("Timeline");

    resize(1500, 500);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
}

//void MainWindow::update()
//{
//    laneScene_->update();
//}
