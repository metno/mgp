#include "mainwindow.h"
#include "leftheaderscene.h"
#include "leftheaderview.h"
#include "lanescene.h"
#include "laneview.h"
#include "topheaderscene.h"
#include "topheaderview.h"
#include "taskmanager.h"
#include "common.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>
#include <QSplitter>
#include <QPushButton>
#include <QKeyEvent>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    // ------------------------

    botSplitter_ = new QSplitter;

    leftHeaderScene_ = new LeftHeaderScene(0, 0, 100, 2000);
    LeftHeaderView *leftHeaderView = new LeftHeaderView(leftHeaderScene_);
    botSplitter_->addWidget(leftHeaderView);

    laneScene_ = new LaneScene(leftHeaderScene_, 2000);
    LaneView *laneView = new LaneView(laneScene_);
    botSplitter_->addWidget(laneView);

    botSplitter_->setSizes(QList<int>() << 100 << 400);

    // ------------------------

    topSplitter_ = new QSplitter;
    QLabel *cornerLabel = new QLabel("MetOrg");
    cornerLabel->setFont(QFont("helvetica", 24));
    cornerLabel->setAlignment(Qt::AlignCenter);
    cornerLabel->setStyleSheet(
                "font-weight:bold; color: red; "
                "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #ffff00, stop: 0.5 #00ffff, stop: 1 #ffff00)");
    topSplitter_->addWidget(cornerLabel);

    topHeaderScene_ = new TopHeaderScene(laneScene_, 50);
    TopHeaderView *topHeaderView = new TopHeaderView(topHeaderScene_);
    topSplitter_->addWidget(topHeaderView);

    // ------------------------

    QSplitter *vsplitter = new QSplitter(Qt::Vertical);
    vsplitter->addWidget(topSplitter_);
    vsplitter->addWidget(botSplitter_);

    mainLayout->addWidget(vsplitter);

    mainLayout->addWidget(new QPushButton("Test"));

    setLayout(mainLayout);
    setWindowTitle("Timeline");

    connect(TaskManager::instance(), SIGNAL(updated()), SLOT(refresh()));
    connect(leftHeaderView, SIGNAL(resized()), SLOT(refresh()));
    connect(topHeaderView, SIGNAL(resized()), SLOT(refresh()));

    connect(laneView->verticalScrollBar(), SIGNAL(valueChanged(int)), leftHeaderView->verticalScrollBar(), SLOT(setValue(int)));
    connect(leftHeaderView->verticalScrollBar(), SIGNAL(valueChanged(int)), laneView->verticalScrollBar(), SLOT(setValue(int)));

    connect(laneView->horizontalScrollBar(), SIGNAL(valueChanged(int)), topHeaderView->horizontalScrollBar(), SLOT(setValue(int)));
    connect(topHeaderView->horizontalScrollBar(), SIGNAL(valueChanged(int)), laneView->horizontalScrollBar(), SLOT(setValue(int)));

    connect(botSplitter_, SIGNAL(splitterMoved(int,int)), SLOT(splitterMoved(int, int)));
    connect(topSplitter_, SIGNAL(splitterMoved(int,int)), SLOT(splitterMoved(int, int)));

    resize(1500, 500);
    topSplitter_->setSizes(botSplitter_->sizes());
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
    topHeaderScene_->refresh();
}

void MainWindow::splitterMoved(int, int)
{
    if (sender() == botSplitter_)
        topSplitter_->setSizes(botSplitter_->sizes());
    else
        botSplitter_->setSizes(topSplitter_->sizes());
}
