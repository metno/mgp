#include "mainwindow.h"
#include "leftheaderscene.h"
#include "leftheaderview.h"
#include "lanescene.h"
#include "laneview.h"
#include "topheaderscene.h"
#include "topheaderview.h"
#include "topview.h"
#include "taskmanager.h"
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
#include <QToolButton>

MainWindow::MainWindow(const QDate &loDate, const QDate &hiDate, QWidget *parent)
    : QWidget(parent)
    , loDate_(loDate)
    , hiDate_(hiDate)
{
    setWindowTitle("MetOrg 0.0.0");

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

    QFrame *cornerFrame = new QFrame;
    QGridLayout *cornerLayout = new QGridLayout;
    cornerFrame->setLayout(cornerLayout);
    cornerLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *appLabel = new QLabel("MetOrg");
    appLabel->setFont(QFont("helvetica", 18));
    appLabel->setAlignment(Qt::AlignCenter);
    appLabel->setStyleSheet(
                "font-weight:bold; color: red; "
                "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #ffff00, stop: 0.5 #00ffff, stop: 1 #ffff00)");
    cornerLayout->addWidget(appLabel, 0, 0, 1, 2);
    cornerLayout->addWidget(new QPushButton("Filter"), 1, 0);
    cornerLayout->addWidget(new QPushButton("Sort"), 1, 1);
    topSplitter_->addWidget(cornerFrame);

    QFrame *topFrame1 = new QFrame;
    topFrame1->setLayout(new QVBoxLayout);
    topFrame1->layout()->setContentsMargins(0, 0, 0, 0);

    QFrame *topFrame2 = new QFrame;
    topFrame2->setLayout(new QHBoxLayout);
    topFrame2->layout()->setContentsMargins(0, 0, 0, 0);

    QDateEdit *loDateEdit = new QDateEdit(loDate);
    loDateEdit->setDisplayFormat("yyyy-MM-dd");
    topFrame2->layout()->addWidget(loDateEdit);

    TopView *topView = new TopView;
    topView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    topFrame2->layout()->addWidget(topView);

    QDateEdit *hiDateEdit = new QDateEdit(hiDate);
    hiDateEdit->setDisplayFormat("yyyy-MM-dd");
    topFrame2->layout()->addWidget(hiDateEdit);

    for (int i = 0; i < 3; ++i) {
        QToolButton *toolButton = new QToolButton;
        toolButton->setText(QString::number(i + 1));
        topFrame2->layout()->addWidget(toolButton);
    }

    topFrame1->layout()->addWidget(topFrame2);

    topHeaderScene_ = new TopHeaderScene(laneScene_, 50);
    TopHeaderView *topHeaderView = new TopHeaderView(topHeaderScene_);
    topFrame1->layout()->addWidget(topHeaderView);

    topSplitter_->addWidget(topFrame1);

    // ------------------------

    QSplitter *vsplitter = new QSplitter(Qt::Vertical);
    vsplitter->addWidget(topSplitter_);
    vsplitter->addWidget(botSplitter_);

    mainLayout->addWidget(vsplitter);

//    QHBoxLayout *botLayout = new QHBoxLayout;
//    QLabel *baseDateLabel = new QLabel(QString("Base date: %1").arg(baseDate_.toString("yyyy-MM-dd")));
//    baseDateLabel->setFont(QFont("helvetica", 18));
//    botLayout->addWidget(baseDateLabel);
//
//    mainLayout->addLayout(botLayout);

    setLayout(mainLayout);

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
//    topSplitter_->setSizes(botSplitter_->sizes());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Escape) || ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Q)))
        close();
}

void MainWindow::showEvent(QShowEvent *)
{
    topSplitter_->setSizes(botSplitter_->sizes());
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
