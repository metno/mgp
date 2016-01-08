#include "timelinecontroller.h"
#include "common.h"
#include <QDateEdit>
#include <QSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QToolButton>
#include <QFormLayout>
#include <QGroupBox>
#include <QSharedPointer>
#include <QSettings>

extern QSharedPointer<QSettings> settings;

TimelineController::TimelineController(
        const QDate &baseDate, const QDate &minBaseDate, const QDate &maxBaseDate,
        int dateSpan, int minDateSpan, int maxDateSpan, QWidget *parent)
    : QWidget(parent)
{
    Q_ASSERT(baseDate >= minBaseDate);
    Q_ASSERT(baseDate <= maxBaseDate);
    Q_ASSERT(dateSpan >= minDateSpan);
    Q_ASSERT(dateSpan <= maxDateSpan);
    Q_ASSERT(minDateSpan > 0);

    baseDateEdit_ = new QDateEdit(baseDate);
    baseDateEdit_->setMinimumDate(minBaseDate);
    baseDateEdit_->setMaximumDate(maxBaseDate);
    baseDateEdit_->setDisplayFormat("yyyy-MM-dd");
    connect(baseDateEdit_, SIGNAL(dateChanged(const QDate &)), SLOT(updateBaseDate()));

    dateSpanSpinBox_ = new QSpinBox;
    dateSpanSpinBox_->setRange(minDateSpan, maxDateSpan);
    dateSpanSpinBox_->setValue(dateSpan);
    connect(dateSpanSpinBox_, SIGNAL(valueChanged(int)), SLOT(updateDateSpan()));

    updateSettings();


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
    QToolButton *todayButton = new QToolButton;
    todayButton->setText("Today");
    connect(todayButton, SIGNAL(clicked()), SLOT(showToday()));
    todayButton->setToolTip("show today's date");
    formLayout->addRow("", todayButton);

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
    updateDateRange(false);
}

void TimelineController::updateDateSpan()
{
    updateDateRange(false);
}

void TimelineController::showToday()
{
    baseDateEdit_->setDate(QDate::currentDate());
    updateDateRange(true);
}

void TimelineController::updateSettings() const
{
    if (settings) {
        settings->setValue("baseDate", baseDateEdit_->date());
        settings->setValue("dateSpan", dateSpanSpinBox_->value());
        settings->sync();
    }
}

void TimelineController::updateDateRange(bool rewind)
{
    updateSettings();
    emit dateRangeUpdated(rewind);
}
