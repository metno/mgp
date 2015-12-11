#include "taskscontroller.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

TasksController::TasksController(QWidget *parent)
    : QWidget(parent)
{
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Visible:", new QLabel("<number>"));
    formLayout->addRow("Sorting:", new QLabel("<...>"));
    formLayout->addRow("Filter:", new QLabel("<...>"));
    formLayout->addRow("Highlighting:", new QLabel("<...>"));

    QGroupBox *groupBox = new QGroupBox("Tasks");
    groupBox->setLayout(formLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(groupBox);
    setLayout(mainLayout);

    layout()->setContentsMargins(0, 0, 0, 0);
}
