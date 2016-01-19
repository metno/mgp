#include "controlpanel.h"
#include "common.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>

ControlPanel &ControlPanel::instance()
{
    static ControlPanel cp;
    return cp;
}

ControlPanel::ControlPanel()
{
    setWindowTitle("Control Panel");
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);


    // --- grid layout -----------------------------------------------
    QGridLayout *gridLayout = new QGridLayout;
    mainLayout->addLayout(gridLayout);

    // add contents to the grid layout ... TBD
    gridLayout->addWidget(new QLabel("aaa"), 0, 0);
    gridLayout->addWidget(new QPushButton("bbb"), 0, 1);
    gridLayout->addWidget(new QLabel("ccc"), 1, 0);
    gridLayout->addWidget(new QPushButton("ddd"), 1, 1);


    mainLayout->addStretch(1);

    // --- bottom panel -----------------------------------------------
    QFrame *botPanel = new QFrame;
    //botPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    botPanel->setLayout(new QHBoxLayout);
    mainLayout->addWidget(botPanel);

    QPushButton *applyButton = new QPushButton("Apply");
    connect(applyButton, SIGNAL(clicked()), SLOT(apply()));
    botPanel->layout()->addWidget(applyButton);

    qobject_cast<QHBoxLayout *>(botPanel->layout())->addStretch(1);

    QPushButton *closeButton = new QPushButton("Close");
    connect(closeButton, SIGNAL(clicked()), SLOT(close()));
    botPanel->layout()->addWidget(closeButton);

    resize(400, 800);
}

void ControlPanel::open()
{
    setVisible(true);
    raise();
}

void ControlPanel::close()
{
    setVisible(false);
}

void ControlPanel::apply()
{
    qDebug() << "apply() ...";
}
