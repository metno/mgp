#include "laneheaderscene.h"
#include "laneheaderview.h"
#include "lanescene.h"
#include "laneview.h"
#include "mainwindow.h"
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
    LaneHeaderView *laneHeaderView = new LaneHeaderView(laneHeaderScene);
    splitter->addWidget(laneHeaderView);

    laneScene_ = new LaneScene(laneHeaderScene, 2000);
    LaneView *laneView = new LaneView(laneScene_);
    splitter->addWidget(laneView);

    splitter->setSizes(QList<int>() << 100 << 400);

    mainLayout->addWidget(splitter);

    mainLayout->addWidget(new QPushButton("Test"));

    setLayout(mainLayout);
    setWindowTitle("Timeline");

    resize(1500, 500);

    update();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
}

void MainWindow::update()
{
    laneScene_->update();
}
