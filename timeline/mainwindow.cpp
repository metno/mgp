#include "mainwindow.h"
#include "laneheaderscene.h"
#include "laneheaderview.h"
#include "lanescene.h"
#include "laneview.h"
#include "timelinescene.h"
#include "timelineview.h"
#include "taskmanager.h"
#include "rolepanel.h"
#include "taskpanel.h"
#include "timelinecontroller.h"
#include "lanescontroller.h"
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
#include <QSharedPointer>
#include <QSettings>

extern QSharedPointer<QSettings> settings;

void MainWindow::init(
        const QDate &baseDate, const QDate &minBaseDate, const QDate &maxBaseDate,
        int dateSpan, int minDateSpan, int maxDateSpan)
{
    baseDate_ = baseDate;
    minBaseDate_ = minBaseDate;
    maxBaseDate_ = maxBaseDate;
    dateSpan_ = dateSpan;
    minDateSpan_ = minDateSpan;
    maxDateSpan_ = maxDateSpan;
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

    hsplitter2_ = new QSplitter;

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
    hsplitter2_->addWidget(cornerFrame);

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

    timelineController_ = new TimelineController(baseDate_, minBaseDate_, maxBaseDate_, dateSpan_, minDateSpan_, maxDateSpan_);
    connect(timelineController_, SIGNAL(dateRangeUpdated(bool)), SLOT(updateDateRange(bool)));
    ctrlFrame->layout()->addWidget(timelineController_);

    lanesController_ = new LanesController();
    ctrlFrame->layout()->addWidget(lanesController_);

    tasksController_ = new TasksController();
    ctrlFrame->layout()->addWidget(tasksController_);

    laneHeaderScene_ = new LaneHeaderScene(2000, 100);
    LaneHeaderView *laneHeaderView = new LaneHeaderView(laneHeaderScene_);
    laneHeaderView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    topFrame->layout()->addWidget(laneHeaderView);

    hsplitter2_->addWidget(topFrame);

    // ------------------------

    hsplitter1_ = new QSplitter;

    laneScene_ = new LaneScene(laneHeaderScene_, baseDate_, dateSpan_);
    LaneView *laneView = new LaneView(laneScene_);

    timelineScene_ = new TimelineScene(laneScene_, 50);
    TimelineView *timelineView = new TimelineView(timelineScene_);

    hsplitter1_->addWidget(timelineView);
    hsplitter1_->addWidget(laneView);

    // ------------------------

    vsplitter1_ = new QSplitter(Qt::Vertical);
    vsplitter1_->addWidget(hsplitter2_);
    vsplitter1_->addWidget(hsplitter1_);

    hsplitter3_ = new QSplitter;
    vsplitter1_->addWidget(hsplitter3_);
    hsplitter3_->addWidget(&RolePanel::instance());
    hsplitter3_->addWidget(&TaskPanel::instance());

    mainLayout->addWidget(vsplitter1_);

    QHBoxLayout *testLayout = new QHBoxLayout;
    QPushButton *testBtn_addNewLane = new QPushButton("Add new lane");
    connect(testBtn_addNewLane, SIGNAL(clicked()), SLOT(addNewLane()));
    testBtn_addNewLane->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    testLayout->addWidget(testBtn_addNewLane);
    testLayout->addStretch(1);
    mainLayout->addLayout(testLayout);

    setLayout(mainLayout);

    connect(&TaskManager::instance(), SIGNAL(updated()), SLOT(updateFromTaskMgr()));
    connect(laneHeaderView, SIGNAL(resized()), SLOT(updateGeometry()));
    connect(timelineView, SIGNAL(resized()), SLOT(updateGeometry()));

    connect(laneView->verticalScrollBar(), SIGNAL(valueChanged(int)), timelineView->verticalScrollBar(), SLOT(setValue(int)));
    connect(timelineView->verticalScrollBar(), SIGNAL(valueChanged(int)), laneView->verticalScrollBar(), SLOT(setValue(int)));

    connect(laneView->horizontalScrollBar(), SIGNAL(valueChanged(int)), laneHeaderView->horizontalScrollBar(), SLOT(setValue(int)));
    connect(laneHeaderView->horizontalScrollBar(), SIGNAL(valueChanged(int)), laneView->horizontalScrollBar(), SLOT(setValue(int)));

    connect(hsplitter1_, SIGNAL(splitterMoved(int,int)), SLOT(updateHSplitter1or2(int, int)));
    connect(hsplitter2_, SIGNAL(splitterMoved(int,int)), SLOT(updateHSplitter1or2(int, int)));
    connect(hsplitter3_, SIGNAL(splitterMoved(int,int)), SLOT(updateHSplitter3(int, int)));
    connect(vsplitter1_, SIGNAL(splitterMoved(int,int)), SLOT(updateVSplitter1(int, int)));

    // set initial window size
    int width_ = 1064;
    int height_ = 989;
    if (settings) {
        bool ok;
        int val = settings->value("width").toInt(&ok);
        if (ok)
           width_ = val;
        val = settings->value("height").toInt(&ok);
        if (ok)
           height_ = val;
    }
    resize(width_, height_);

    hsplitter1_->setSizes(loadSplitterSizesFromSettings("hsplitter1", QList<int>() << 178 << 858));
    vsplitter1_->setSizes(loadSplitterSizesFromSettings("vsplitter1", QList<int>() << 257 << 487 << 178));
    hsplitter3_->setSizes(loadSplitterSizesFromSettings("hsplitter3", QList<int>() << 518 << 518));
}

bool MainWindow::isInit_ = false;
QDate MainWindow::baseDate_ = QDate();
QDate MainWindow::minBaseDate_ = QDate();
QDate MainWindow::maxBaseDate_ = QDate();
int MainWindow::dateSpan_ = -1;
int MainWindow::minDateSpan_ = -1;
int MainWindow::maxDateSpan_ = -1;

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
    hsplitter2_->setSizes(hsplitter1_->sizes());

    // set initial scaling
    qreal hscale = 3;
    qreal vscale = 0.01;
    if (settings) {
        bool ok;
        qreal val = settings->value("hscale").toReal(&ok);
        if (ok)
           hscale = val;
        val = settings->value("vscale").toReal(&ok);
        if (ok)
           vscale = val;
    }
    qobject_cast<LaneView *>(laneScene_->views().first())->updateScale(hscale, vscale);
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    // update settings
    if (settings) {
        settings->setValue("width", width());
        settings->setValue("height", height());
    }
}

void MainWindow::updateFromTaskMgr()
{
    laneHeaderScene_->updateFromTaskMgr();
    laneScene_->updateFromTaskMgr();
    timelineScene_->updateFromTaskMgr();
}

void MainWindow::updateGeometry()
{
    laneHeaderScene_->updateGeometryAndContents();
    laneScene_->updateGeometryAndContents();
    timelineScene_->updateGeometry();
}

QList<int> MainWindow::loadSplitterSizesFromSettings(const QString &name, const QList<int> &defaultSizes)
{
    if (!settings)
        return defaultSizes;

    QList<int> sizes;
    const int nsizes = settings->beginReadArray(name);
    if (nsizes != defaultSizes.size()) {
        settings->endArray();
        return defaultSizes;
    }
    for (int i = 0; i < nsizes; ++i) {
        settings->setArrayIndex(i);
        bool ok = false;
        const int size = settings->value("size").toInt(&ok);
        if (ok) {
            sizes.append(size);
        } else {
            settings->endArray();
            return defaultSizes;
        }
    }

    settings->endArray();
    return sizes;
}

void MainWindow::updateSplitterSettings(const QSplitter *splitter, const QString &name)
{
    if (!settings)
        return;

    settings->beginWriteArray(name);
    int index = 0;
    foreach (int size, splitter->sizes()) {
        settings->setArrayIndex(index++);
        settings->setValue("size", size);
    }
    settings->endArray();

    settings->sync();
}

void MainWindow::updateAllSplitterSettings()
{
    updateSplitterSettings(hsplitter1_, "hsplitter1");
    updateSplitterSettings(hsplitter3_, "hsplitter3");
    updateSplitterSettings(vsplitter1_, "vsplitter1");
}

void MainWindow::updateHSplitter1or2(int, int)
{
    if (sender() == hsplitter1_) {
        hsplitter2_->setSizes(hsplitter1_->sizes());
        if (hsplitter2_->sizes() != hsplitter1_->sizes())
            hsplitter1_->setSizes(hsplitter2_->sizes());
    } else {
        hsplitter1_->setSizes(hsplitter2_->sizes());
        if (hsplitter1_->sizes() != hsplitter2_->sizes())
            hsplitter2_->setSizes(hsplitter1_->sizes());
    }

    updateAllSplitterSettings();
}

void MainWindow::updateHSplitter3(int, int)
{
    updateAllSplitterSettings();
}

void MainWindow::updateVSplitter1(int, int)
{
    updateAllSplitterSettings();
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

void MainWindow::addNewLane()
{
    // ### For now, assume that a lane always represents a role. Later, this will be just a special type of lane
    // (typically with its filter set to "role=...")
    TaskManager::instance().addNewRole();
}
