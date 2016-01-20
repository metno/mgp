#include "controlpanel.h"
#include "common.h"
#include "mainwindow.h"
#include "glwidget.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPointF>
#include <QButtonGroup>

Filter::Filter(Type type, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox)
    : type_(type)
    , enabledCheckBox_(enabledCheckBox)
    , currCheckBox_(currCheckBox)
{
    connect(enabledCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    connect(currCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
}

QString Filter::typeName(Type type)
{
    switch (type) {
    case  E_OF: return  "E OF";
    case  W_OF: return  "W OF";
    case  N_OF: return  "N OF";
    case  S_OF: return  "S OF";
    case NE_OF: return "NE OF";
    case NW_OF: return "NW OF";
    case SE_OF: return "SE OF";
    case SW_OF: return "SW OF";
    default: return "ERROR!";
    }
}

void Filter::updateGLWidget()
{
    MainWindow::instance().glWidget()->updateFilter(type_, enabledCheckBox_->isChecked(), currCheckBox_->isChecked(), value());
}

LonOrLatFilter::LonOrLatFilter(
        Type type, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox, QDoubleSpinBox *valSpinBox, double defaultValue)
    : Filter(type, enabledCheckBox, currCheckBox)
    , valSpinBox_(valSpinBox)
{
    if ((type == W_OF) || (type == E_OF)) {
        valSpinBox_->setMinimum(-180);
        valSpinBox_->setMaximum(180);
    } else {
        valSpinBox_->setMinimum(-90);
        valSpinBox_->setMaximum(90);
    }
    valSpinBox_->setValue(defaultValue);
    connect(valSpinBox_, SIGNAL(valueChanged(double)), SLOT(updateGLWidget()));
}

Filter *LonOrLatFilter::create(QGridLayout *layout, int row, Type type, double defaultValue)
{
    QDoubleSpinBox *valSpinBox = new QDoubleSpinBox;
    valSpinBox->setDecimals(3);
    LonOrLatFilter *filter = new LonOrLatFilter(type, new QCheckBox, new QCheckBox, valSpinBox, defaultValue);

    QLabel *typeLabel = new QLabel(typeName(filter->type_));
    typeLabel->setStyleSheet("font-family:mono");
    layout->addWidget(typeLabel, row, 0, Qt::AlignRight);
    layout->addWidget(filter->enabledCheckBox_, row, 1, Qt::AlignHCenter);
    layout->addWidget(filter->currCheckBox_, row, 2, Qt::AlignHCenter);

    QFrame *valFrame = new QFrame;
    valFrame->setLayout(new QHBoxLayout);
    valFrame->layout()->addWidget(new QLabel(QString("%1:").arg(((type == E_OF) || (type == W_OF)) ? "lon" : "lat")));
    valFrame->layout()->addWidget(filter->valSpinBox_);
    qobject_cast<QHBoxLayout *>(valFrame->layout())->addStretch(1);
    layout->addWidget(valFrame, row, 3);

    return filter;
}

QVariant LonOrLatFilter::value() const
{
    return valSpinBox_->value();
}

FreeLineFilter::FreeLineFilter(
        Type type, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox,
        QDoubleSpinBox *lon1SpinBox, QDoubleSpinBox *lat1SpinBox, QDoubleSpinBox *lon2SpinBox, QDoubleSpinBox *lat2SpinBox,
        const QLineF &defaultValue)
    : Filter(type, enabledCheckBox, currCheckBox)
    , lon1SpinBox_(lon1SpinBox)
    , lat1SpinBox_(lat1SpinBox)
    , lon2SpinBox_(lon2SpinBox)
    , lat2SpinBox_(lat2SpinBox)
{
    lon1SpinBox_->setValue(defaultValue.x1());
    lat1SpinBox_->setValue(defaultValue.y1());
    lon2SpinBox_->setValue(defaultValue.x2());
    lat2SpinBox_->setValue(defaultValue.y2());
    connect(lon1SpinBox_, SIGNAL(valueChanged(double)), SLOT(updateGLWidget()));
    connect(lat1SpinBox_, SIGNAL(valueChanged(double)), SLOT(updateGLWidget()));
    connect(lon2SpinBox_, SIGNAL(valueChanged(double)), SLOT(updateGLWidget()));
    connect(lat2SpinBox_, SIGNAL(valueChanged(double)), SLOT(updateGLWidget()));
}

Filter *FreeLineFilter::create(QGridLayout *layout, int row, Type type, const QLineF &defaultValue)
{
    QDoubleSpinBox *lon1SpinBox = new QDoubleSpinBox; lon1SpinBox->setDecimals(3);
    QDoubleSpinBox *lat1SpinBox = new QDoubleSpinBox; lat1SpinBox->setDecimals(3);
    QDoubleSpinBox *lon2SpinBox = new QDoubleSpinBox; lon2SpinBox->setDecimals(3);
    QDoubleSpinBox *lat2SpinBox = new QDoubleSpinBox; lat2SpinBox->setDecimals(3);
    FreeLineFilter *filter = new FreeLineFilter(
                type, new QCheckBox, new QCheckBox, lon1SpinBox, lat1SpinBox, lon2SpinBox, lat2SpinBox, defaultValue);

    QLabel *typeLabel = new QLabel(typeName(filter->type_));
    typeLabel->setStyleSheet("font-family:mono");
    layout->addWidget(typeLabel, row, 0, Qt::AlignRight);
    layout->addWidget(filter->enabledCheckBox_, row, 1, Qt::AlignHCenter);
    layout->addWidget(filter->currCheckBox_, row, 2, Qt::AlignHCenter);

    QFrame *valFrame = new QFrame;
    valFrame->setLayout(new QHBoxLayout);
    valFrame->layout()->addWidget(new QLabel("lon 1:"));
    valFrame->layout()->addWidget(lon1SpinBox);
    valFrame->layout()->addWidget(new QLabel("lat 1:"));
    valFrame->layout()->addWidget(lat1SpinBox);
    valFrame->layout()->addWidget(new QLabel("lon 2:"));
    valFrame->layout()->addWidget(lon2SpinBox);
    valFrame->layout()->addWidget(new QLabel("lat 2:"));
    valFrame->layout()->addWidget(lat2SpinBox);
    layout->addWidget(valFrame, row, 3);

    return filter;
}

QVariant FreeLineFilter::value() const
{
    return QLineF(QPointF(lon1SpinBox_->value(), lat1SpinBox_->value()), QPointF(lon2SpinBox_->value(), lat2SpinBox_->value()));
}

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

    // --- BEGIN base polygon section -------------------------------------------
    // --- END base polygon section -------------------------------------------


    // --- BEGIN filter section -------------------------------------------
    QGridLayout *filterLayout = new QGridLayout;
    mainLayout->addLayout(filterLayout);

    // header
    filterLayout->addWidget(new QLabel("Filters:"), 0, 0, 1, 5);
    filterLayout->itemAtPosition(0, 0)->widget()->setStyleSheet("font-weight:bold; font-size:16px");
    filterLayout->addWidget(new QLabel("Type"), 1, 0, Qt::AlignHCenter);
    filterLayout->addWidget(new QLabel("Enabled"), 1, 1, Qt::AlignHCenter);
    filterLayout->addWidget(new QLabel("Current"), 1, 2, Qt::AlignHCenter);
    filterLayout->addWidget(new QLabel("Value"), 1, 3, 1, 2, Qt::AlignHCenter);
    for (int i = 0; i < 4; ++i)
        filterLayout->itemAtPosition(1, i)->widget()->setStyleSheet("font-weight:bold");

    // lon|lat filters (default values arbitrarily chosen for now)
    filters_.insert(Filter::E_OF, LonOrLatFilter::create(filterLayout, 2, Filter::E_OF, 7.2));
    filters_.insert(Filter::W_OF, LonOrLatFilter::create(filterLayout, 3, Filter::W_OF, 9.5));
    filters_.insert(Filter::N_OF, LonOrLatFilter::create(filterLayout, 4, Filter::N_OF, 60.3));
    filters_.insert(Filter::S_OF, LonOrLatFilter::create(filterLayout, 5, Filter::S_OF, 62.8));

    // line filters (default values arbitrarily chosen for now)
    filters_.insert(Filter::NE_OF, FreeLineFilter::create(filterLayout, 6, Filter::NE_OF, QLineF(QPointF(4, 70), QPointF(10, 50))));
    filters_.insert(Filter::NW_OF, FreeLineFilter::create(filterLayout, 7, Filter::NW_OF, QLineF(QPointF(4, 50), QPointF(10, 70))));
    filters_.insert(Filter::SE_OF, FreeLineFilter::create(filterLayout, 8, Filter::SE_OF, QLineF(QPointF(4, 50), QPointF(10, 70))));
    filters_.insert(Filter::SW_OF, FreeLineFilter::create(filterLayout, 9, Filter::SW_OF, QLineF(QPointF(4, 70), QPointF(10, 50))));

    // ensure exclusive/radio behavior for the 'current' state
    QButtonGroup *currBtnGroup = new QButtonGroup;
    currBtnGroup->setExclusive(true);
    foreach (Filter *filter, filters_)
        currBtnGroup->addButton(filter->currCheckBox_);

    // --- END filter section -------------------------------------------


    mainLayout->addStretch(1);


    // --- BEGIN bottom panel -----------------------------------------------
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
    // --- END bottom panel -----------------------------------------------
}

void ControlPanel::keyPressEvent(QKeyEvent *event)
{
    MainWindow::instance().handleKeyPressEvent(event);
}

void ControlPanel::open()
{
    setVisible(true);
    raise();
}

void ControlPanel::apply()
{
    qDebug() << "apply() ...";
    const int typeNameWidth = 5;
    foreach (Filter *filter, filters_) {
        qDebug() << "  type:" << QString("%1").arg(Filter::typeName(filter->type_), typeNameWidth).toLatin1().data()
                 << ", enabled:" << filter->enabledCheckBox_->isChecked() << ", value:" << filter->value();
    }

    // compute resulting polygon(s) from base polygon and enabled filters ... TBD
}

void ControlPanel::close()
{
    setVisible(false);
}
