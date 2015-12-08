#ifndef TIMELINECONTOLLER_H
#define TIMELINECONTOLLER_H

#include <QWidget>
#include <QDate>

class QDateEdit;
class QSpinBox;

class TimelineController : public QWidget
{
    Q_OBJECT

public:
    TimelineController(const QDate &, int, QWidget * = 0);
    QDate baseDate() const;
    int dateSpan() const;

private:
    QDateEdit *baseDateEdit_;
    QSpinBox *dateSpanSpinBox_;

private slots:
    void updateBaseDate();
    void updateDateSpan();
    void showToday();

signals:
    void updateDateRange(bool);
};

#endif // TIMELINECONTOLLER_H
