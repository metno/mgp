#include "mainwindow.h"
#include "rolesscene.h"
#include "rolesview.h"
#include "lanescene.h"
#include "laneview.h"
#include "timelinescene.h"
#include "timelineview.h"
#include "taskmanager.h"
#include "taskpanel.h"
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

void MainWindow::init(const QDate &baseDate, int dateSpan)
{
    baseDate_ = baseDate;
    dateSpan_ = dateSpan;
    isInit_ = true;
}

MainWindow &MainWindow::instance()
{
    if (!isInit_)
        qFatal("MainWindow not initialized");
    static MainWindow mw;
    return mw;
}

MainWindow::MainWindow()
{
    setWindowTitle("MetOrg 0.0.0");

    QVBoxLayout *mainLayout = new QVBoxLayout;

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

    timelineController_ = new TimelineController(baseDate_, dateSpan_);
    connect(timelineController_, SIGNAL(updateDateRange(bool)), SLOT(updateDateRange(bool)));
    ctrlFrame->layout()->addWidget(timelineController_);

    rolesController_ = new RolesController();
    ctrlFrame->layout()->addWidget(rolesController_);

    tasksController_ = new TasksController();
    ctrlFrame->layout()->addWidget(tasksController_);

    rolesScene_ = new RolesScene(2000, 100);
    RolesView *rolesView = new RolesView(rolesScene_);
    rolesView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    topFrame->layout()->addWidget(rolesView);

    topSplitter_->addWidget(topFrame);

    // ------------------------

    botSplitter_ = new QSplitter;

    laneScene_ = new LaneScene(rolesScene_, baseDate_, dateSpan_);
    LaneView *laneView = new LaneView(laneScene_);

    timelineScene_ = new TimelineScene(laneScene_, 50);
    TimelineView *timelineView = new TimelineView(timelineScene_);

    botSplitter_->addWidget(timelineView);
    botSplitter_->addWidget(laneView);

    botSplitter_->setSizes(QList<int>() << 100 << 400);

    // ------------------------

    QSplitter *vsplitter = new QSplitter(Qt::Vertical);
    vsplitter->addWidget(topSplitter_);
    vsplitter->addWidget(botSplitter_);
    vsplitter->addWidget(&TaskPanel::instance());

    mainLayout->addWidget(vsplitter);

    QHBoxLayout *testLayout = new QHBoxLayout;
    QPushButton *testBtn_add5Roles = new QPushButton("Add 5 roles");
    connect(testBtn_add5Roles, SIGNAL(clicked()), SLOT(add5Roles()));
    testBtn_add5Roles->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    testLayout->addWidget(testBtn_add5Roles);
    QPushButton *testBtn_test2 = new QPushButton("Test 2");
    connect(testBtn_test2, SIGNAL(clicked()), SLOT(test2()));
    testBtn_test2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    testLayout->addWidget(testBtn_test2);
    testLayout->addStretch(1);
    mainLayout->addLayout(testLayout);

    setLayout(mainLayout);

    connect(&TaskManager::instance(), SIGNAL(updated()), SLOT(updateFromTaskMgr()));
    connect(rolesView, SIGNAL(resized()), SLOT(updateGeometry()));
    connect(timelineView, SIGNAL(resized()), SLOT(updateGeometry()));

    connect(laneView->verticalScrollBar(), SIGNAL(valueChanged(int)), timelineView->verticalScrollBar(), SLOT(setValue(int)));
    connect(timelineView->verticalScrollBar(), SIGNAL(valueChanged(int)), laneView->verticalScrollBar(), SLOT(setValue(int)));

    connect(laneView->horizontalScrollBar(), SIGNAL(valueChanged(int)), rolesView->horizontalScrollBar(), SLOT(setValue(int)));
    connect(rolesView->horizontalScrollBar(), SIGNAL(valueChanged(int)), laneView->horizontalScrollBar(), SLOT(setValue(int)));

    connect(botSplitter_, SIGNAL(splitterMoved(int,int)), SLOT(updateSplitters(int, int)));
    connect(topSplitter_, SIGNAL(splitterMoved(int,int)), SLOT(updateSplitters(int, int)));

    resize(1000, 700);
}

bool MainWindow::isInit_ = false;
QDate MainWindow::baseDate_ = QDate();
int MainWindow::dateSpan_ = -1;

void MainWindow::handleKeyPressEvent(QKeyEvent *event)
{
    keyPressEvent(event); // for now, propagate to private handler without filtering
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
    qobject_cast<LaneView *>(laneScene_->views().first())->updateScale(1.5, 0.01);
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
    if (sender() == botSplitter_) {
        topSplitter_->setSizes(botSplitter_->sizes());
        if (topSplitter_->sizes() != botSplitter_->sizes())
            botSplitter_->setSizes(topSplitter_->sizes());
    } else {
        botSplitter_->setSizes(topSplitter_->sizes());
        if (botSplitter_->sizes() != topSplitter_->sizes())
            topSplitter_->setSizes(botSplitter_->sizes());
    }
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

void MainWindow::add5Roles()
{
    TaskManager::instance().add5Roles();
}

void MainWindow::test2()
{
    qDebug() << "test2() (does nothing, just an example)";
    // open modal dialog to get task attributes (start/end/color/title/summary/description), then, if Ok, call:
    //TaskManager::instance()->addTask(<task attributes>);
}
