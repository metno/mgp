#include "timelinecontroller.h"
#include <QDateEdit>
#include <QSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QToolButton>
#include <QFormLayout>
#include <QGroupBox>

TimelineController::TimelineController(const QDate &baseDate, int dateSpan, QWidget *parent)
    : QWidget(parent)
{
    const int minDateSpan = 1;
    const int maxDateSpan = 10;
    if ((dateSpan < minDateSpan) || (dateSpan > maxDateSpan))
        qWarning("date span (%d) outside valid range ([%d, %d])", dateSpan, minDateSpan, maxDateSpan);
    dateSpan = qMin(qMax(dateSpan, minDateSpan), maxDateSpan);

    // ------------------------

    baseDateEdit_ = new QDateEdit(baseDate);
    baseDateEdit_->setDisplayFormat("yyyy-MM-dd");
    connect(baseDateEdit_, SIGNAL(dateChanged(const QDate &)), SLOT(updateBaseDate()));

    dateSpanSpinBox_ = new QSpinBox;
    dateSpanSpinBox_->setRange(minDateSpan, maxDateSpan);
    dateSpanSpinBox_->setValue(dateSpan);
    connect(dateSpanSpinBox_, SIGNAL(valueChanged(int)), SLOT(updateDateSpan()));

//    {
//        QToolButton *toolButton = new QToolButton;
//        toolButton->setText("T");
//        qobject_cast<QHBoxLayout *>(layout())->insertWidget(0, toolButton);
//        connect(toolButton, SIGNAL(clicked()), SLOT(showToday()));
//        toolButton->setToolTip("show today's date");
//    }

//    for (int i = 0; i < 3; ++i) {
//        QToolButton *toolButton = new QToolButton;
//        toolButton->setText(QString('A' + i));
//        layout()->addWidget(toolButton);
//    }

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Base date:", baseDateEdit_);
    formLayout->addRow("Date span:", dateSpanSpinBox_);
    formLayout->addRow("Filter:", new QLabel("<...>"));
    formLayout->addRow("Highlighting:", new QLabel("<...>"));

    QGroupBox *groupBox = new QGroupBox("Timeline");
    groupBox->setLayout(formLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(groupBox);
    setLayout(mainLayout);

    layout()->setContentsMargins(0, 0, 0, 0);
}

QDate TimelineController::baseDate() const
{
    return baseDateEdit_->date();
}

int TimelineController::dateSpan() const
{
    return dateSpanSpinBox_->value();
}

void TimelineController::updateBaseDate()
{
    emit updateDateRange(false);
}

void TimelineController::updateDateSpan()
{
    emit updateDateRange(false);
}

void TimelineController::showToday()
{
    baseDateEdit_->setDate(QDate::currentDate());
    emit updateDateRange(true);
}
