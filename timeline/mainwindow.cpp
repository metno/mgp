#include "mainwindow.h"
#include "rolesscene.h"
#include "rolesview.h"
#include "lanescene.h"
#include "laneview.h"
#include "timelinescene.h"
#include "timelineview.h"
#include "taskmanager.h"
#include "timelinecontroller.h"
#include "rolescontroller.h"
#include "taskscontroller.h"
#include "common.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QSplitter>
#include <QPushButton>
#include <QKeyEvent>
#include <QScrollBar>
#include <QFrame>
#include <QDateEdit>
#include <QSpinBox>
#include <QToolButton>
#include <QScrollBar>

MainWindow::MainWindow(const QDate &baseDate, int dateSpan, QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("MetOrg 0.0.0");

    QVBoxLayout *mainLayout = new QVBoxLayout;

    // ------------------------

    botSplitter_ = new QSplitter;

    rolesScene_ = new RolesScene(0, 0, 100, 2000);
    RolesView *rolesView = new RolesView(rolesScene_);
    botSplitter_->addWidget(rolesView);

    laneScene_ = new LaneScene(rolesScene_, baseDate, dateSpan);
    LaneView *laneView = new LaneView(laneScene_);
    botSplitter_->addWidget(laneView);

    botSplitter_->setSizes(QList<int>() << 100 << 400);

    // ------------------------

    topSplitter_ = new QSplitter;

    QFrame *cornerFrame = new QFrame;
    QGridLayout *cornerLayout = new QGridLayout;
    cornerFrame->setLayout(cornerLayout);
    cornerLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *appLabel = new QLabel("MetOrg");
    appLabel->setFont(QFont("helvetica", 18));
    appLabel->setAlignment(Qt::AlignCenter);
    appLabel->setStyleSheet(
                "font-weight:normal; color: #000000; "
                "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #bbbbff, stop: 0.5 #bbeeff, stop: 1 #bbbbff)");
    cornerLayout->addWidget(appLabel, 0, 0, 1, 2);
    cornerLayout->addWidget(new QPushButton("Filter"), 1, 0);
    cornerLayout->addWidget(new QPushButton("Sort"), 1, 1);
    topSplitter_->addWidget(cornerFrame);

    QFrame *topFrame = new QFrame;
    topFrame->setLayout(new QVBoxLayout);
    topFrame->layout()->setContentsMargins(0, 0, 0, 0);

    QFrame *ctrlFrame = new QFrame;
    ctrlFrame->setLayout(new QHBoxLayout);
    ctrlFrame->layout()->setContentsMargins(0, 0, 0, 0);
    topFrame->layout()->addWidget(ctrlFrame);

    // ### put this somewhere else (it doesn't belong only to timeline controller, since the reset applies to both dimensions)
    //    {
    //        QToolButton *toolButton = new QToolButton;
    //        toolButton->setText("Z");
    //        qobject_cast<QHBoxLayout *>(topFrame2->layout())->insertWidget(1, toolButton);
    //        connect(toolButton, SIGNAL(clicked()), SLOT(resetZooming()));
    //        toolButton->setToolTip("reset zooming");
    //    }

    timelineController_ = new TimelineController(baseDate, dateSpan);
    connect(timelineController_, SIGNAL(updateDateRange(bool)), SLOT(updateDateRange(bool)));
    ctrlFrame->layout()->addWidget(timelineController_);

    rolesController_ = new RolesController();
    ctrlFrame->layout()->addWidget(rolesController_);

    tasksController_ = new TasksController();
    ctrlFrame->layout()->addWidget(tasksController_);

    timelineScene_ = new TimelineScene(laneScene_, 50);
    TimelineView *timelineView = new TimelineView(timelineScene_);
    topFrame->layout()->addWidget(timelineView);

    topSplitter_->addWidget(topFrame);

    // ------------------------

    QSplitter *vsplitter = new QSplitter(Qt::Vertical);
    vsplitter->addWidget(topSplitter_);
    vsplitter->addWidget(botSplitter_);

    mainLayout->addWidget(vsplitter);

    QHBoxLayout *testLayout = new QHBoxLayout;
    QPushButton *testBtn_add5Roles = new QPushButton("Add 5 roles");
    connect(testBtn_add5Roles, SIGNAL(clicked()), SLOT(test1()));
    testBtn_add5Roles->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    testLayout->addWidget(testBtn_add5Roles);
    QPushButton *testButton2 = new QPushButton("Test 2");
    testButton2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    testLayout->addWidget(testButton2);
    testLayout->addStretch(1);
    mainLayout->addLayout(testLayout);

    setLayout(mainLayout);

    connect(TaskManager::instance(), SIGNAL(updated()), SLOT(updateFromTaskMgr()));
    connect(rolesView, SIGNAL(resized()), SLOT(updateGeometry()));
    connect(timelineView, SIGNAL(resized()), SLOT(updateGeometry()));

    connect(laneView->verticalScrollBar(), SIGNAL(valueChanged(int)), rolesView->verticalScrollBar(), SLOT(setValue(int)));
    connect(rolesView->verticalScrollBar(), SIGNAL(valueChanged(int)), laneView->verticalScrollBar(), SLOT(setValue(int)));

    connect(laneView->horizontalScrollBar(), SIGNAL(valueChanged(int)), timelineView->horizontalScrollBar(), SLOT(setValue(int)));
    connect(timelineView->horizontalScrollBar(), SIGNAL(valueChanged(int)), laneView->horizontalScrollBar(), SLOT(setValue(int)));

    connect(botSplitter_, SIGNAL(splitterMoved(int,int)), SLOT(updateSplitters(int, int)));
    connect(topSplitter_, SIGNAL(splitterMoved(int,int)), SLOT(updateSplitters(int, int)));

    resize(1500, 500);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Escape) || ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Q)))
        close();
}

void MainWindow::showEvent(QShowEvent *)
{
    topSplitter_->setSizes(botSplitter_->sizes());

    // set initial scaling
    qobject_cast<LaneView *>(laneScene_->views().first())->updateScale(0.01, 1.0);
}

void MainWindow::updateFromTaskMgr()
{
    rolesScene_->updateFromTaskMgr();
    laneScene_->updateFromTaskMgr();
    timelineScene_->updateFromTaskMgr();
}

void MainWindow::updateGeometry()
{
    rolesScene_->updateGeometry();
    laneScene_->updateGeometry();
    timelineScene_->updateGeometry();
}

void MainWindow::updateSplitters(int, int)
{
    if (sender() == botSplitter_)
        topSplitter_->setSizes(botSplitter_->sizes());
    else
        botSplitter_->setSizes(topSplitter_->sizes());
}

void MainWindow::updateDateRange(bool rewind)
{
    laneScene_->setDateRange(timelineController_->baseDate(), timelineController_->dateSpan());
    if (rewind) {
        QScrollBar *hsbar = laneScene_->views().first()->horizontalScrollBar();
        hsbar->setValue(hsbar->minimum());
    }
}

void MainWindow::resetZooming()
{
    qobject_cast<LaneView *>(laneScene_->views().first())->updateScale(1, 1);
}

void MainWindow::test1()
{
    TaskManager::instance()->add5Roles();
}
