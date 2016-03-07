#include "controlpanel.h"
#include "common.h"
#include "mainwindow.h"
#include "glwidget.h"
#include "enor_fir.h"
#include "textedit.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPointF>
#include <QButtonGroup>
#include <QTextEdit>
#include <QComboBox>
#include <QSlider>
#include <QBitArray>
#include <QStack>
#include <QMessageBox>
#include <QTabWidget>
#include <QDialogButtonBox>

FilterControlBase::FilterControlBase(mgp::FilterBase *filter, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox)
    : filter_(mgp::Filter(filter))
    , enabledCheckBox_(enabledCheckBox)
    , currCheckBox_(currCheckBox)
    , dragged_(false)
{
    connect(enabledCheckBox_, SIGNAL(stateChanged(int)), &ControlPanel::instance(), SLOT(updateGLWidget()));
    connect(enabledCheckBox_, SIGNAL(stateChanged(int)), &ControlPanel::instance(), SLOT(updateFilterTabTexts()));
    connect(currCheckBox_, SIGNAL(stateChanged(int)), &ControlPanel::instance(), SLOT(updateGLWidget()));
}

WithinFilterControl::WithinFilterControl(
        mgp::WithinFilter *filter, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox)
    : FilterControlBase(filter, enabledCheckBox, currCheckBox)
{
}

FilterControlBase *WithinFilterControl::create(QGridLayout *layout, int row, mgp::WithinFilter *filter)
{
    WithinFilterControl *filterControl = new WithinFilterControl(filter, new QCheckBox, new QCheckBox);

    QLabel *typeLabel = new QLabel("WI");
    typeLabel->setStyleSheet("font-family:mono");
    layout->addWidget(typeLabel, row, 0, Qt::AlignRight);
    layout->addWidget(filterControl->enabledCheckBox_, row, 1, Qt::AlignHCenter);
    layout->addWidget(filterControl->currCheckBox_, row, 2, Qt::AlignHCenter);

    QFrame *valFrame = new QFrame;
    valFrame->setLayout(new QHBoxLayout);
    valFrame->layout()->addWidget(new QLabel("<coordinates accessible on sphere only>"));
    qobject_cast<QHBoxLayout *>(valFrame->layout())->addStretch(1);
    layout->addWidget(valFrame, row, 3);

    return filterControl;
}

void WithinFilterControl::update()
{
    // no controls to update (all editing occurs on sphere only)
}

QVariant WithinFilterControl::value() const
{
    return QVariant(); // for now
}

bool WithinFilterControl::startDragging(const mgp::Point &)
{
    return false;
}

void WithinFilterControl::updateDragging(const mgp::Point &)
{
}

LonOrLatFilterControl::LonOrLatFilterControl(
        mgp::LonOrLatFilter *filter, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox, QDoubleSpinBox *valSpinBox)
    : FilterControlBase(filter, enabledCheckBox, currCheckBox)
    , lonOrLatFilter_(QSharedPointer<mgp::LonOrLatFilter>(filter))
    , valSpinBox_(valSpinBox)
{
    if (filter->isLonFilter()) {
        valSpinBox_->setMinimum(-180);
        valSpinBox_->setMaximum(180);
    } else {
        valSpinBox_->setMinimum(-90);
        valSpinBox_->setMaximum(90);
    }
    valSpinBox_->setValue(RAD2DEG(filter->value()));
    connect(valSpinBox_, SIGNAL(valueChanged(double)), SLOT(handleSpinBoxValueChanged()));
}

FilterControlBase *LonOrLatFilterControl::create(QGridLayout *layout, int row, mgp::LonOrLatFilter *filter)
{
    QDoubleSpinBox *valSpinBox = new QDoubleSpinBox;
    valSpinBox->setDecimals(3);
    LonOrLatFilterControl *filterControl = new LonOrLatFilterControl(filter, new QCheckBox, new QCheckBox, valSpinBox);

    QLabel *typeLabel = new QLabel(QString("%1 OF").arg(filter->directionName()));
    typeLabel->setStyleSheet("font-family:mono");
    layout->addWidget(typeLabel, row, 0, Qt::AlignRight);
    layout->addWidget(filterControl->enabledCheckBox_, row, 1, Qt::AlignHCenter);
    layout->addWidget(filterControl->currCheckBox_, row, 2, Qt::AlignHCenter);

    QFrame *valFrame = new QFrame;
    valFrame->setLayout(new QHBoxLayout);
    valFrame->layout()->addWidget(new QLabel(QString("%1:").arg(filter->isLonFilter() ? "lon" : "lat")));
    valFrame->layout()->addWidget(filterControl->valSpinBox_);
    qobject_cast<QHBoxLayout *>(valFrame->layout())->addStretch(1);
    layout->addWidget(valFrame, row, 3);

    return filterControl;
}

void LonOrLatFilterControl::update()
{
   valSpinBox_->setValue(RAD2DEG(lonOrLatFilter_->value()));
}

QVariant LonOrLatFilterControl::value() const
{
    return valSpinBox_->value();
}

bool LonOrLatFilterControl::startDragging(const mgp::Point &point)
{
    const double val = lonOrLatFilter_->isLonFilter() ? point.first : point.second;
    valSpinBox_->setValue(RAD2DEG(val));
    lonOrLatFilter_->setValue(val);
    dragged_ = true;
    return true;
}

void LonOrLatFilterControl::updateDragging(const mgp::Point &point)
{
    Q_ASSERT(dragged_);
    const double val = lonOrLatFilter_->isLonFilter() ? point.first : point.second;
    valSpinBox_->setValue(RAD2DEG(val));
    lonOrLatFilter_->setValue(val);
}

void LonOrLatFilterControl::handleSpinBoxValueChanged()
{
    lonOrLatFilter_->setValue(DEG2RAD(valSpinBox_->value()));
    ControlPanel::instance().updateGLWidget();
}

FreeLineFilterControl::FreeLineFilterControl(
        mgp::FreeLineFilter *filter, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox,
        QDoubleSpinBox *lon1SpinBox, QDoubleSpinBox *lat1SpinBox, QDoubleSpinBox *lon2SpinBox, QDoubleSpinBox *lat2SpinBox)
    : FilterControlBase(filter, enabledCheckBox, currCheckBox)
    , freeLineFilter_(QSharedPointer<mgp::FreeLineFilter>(filter))
    , lon1SpinBox_(lon1SpinBox)
    , lat1SpinBox_(lat1SpinBox)
    , lon2SpinBox_(lon2SpinBox)
    , lat2SpinBox_(lat2SpinBox)
    , firstEndpointDragged_(false)
{
    lon1SpinBox_->setMinimum(-180);
    lon1SpinBox_->setMaximum(180);
    lon1SpinBox_->setValue(RAD2DEG(filter->lon1()));

    lat1SpinBox_->setMinimum(-90);
    lat1SpinBox_->setMaximum(90);
    lat1SpinBox_->setValue(RAD2DEG(filter->lat1()));

    lon2SpinBox_->setMinimum(-180);
    lon2SpinBox_->setMaximum(180);
    lon2SpinBox_->setValue(RAD2DEG(filter->lon2()));

    lat2SpinBox_->setMinimum(-90);
    lat2SpinBox_->setMaximum(90);
    lat2SpinBox_->setValue(RAD2DEG(filter->lat2()));

    connect(lon1SpinBox_, SIGNAL(valueChanged(double)), SLOT(handleSpinBoxValueChanged()));
    connect(lat1SpinBox_, SIGNAL(valueChanged(double)), SLOT(handleSpinBoxValueChanged()));
    connect(lon2SpinBox_, SIGNAL(valueChanged(double)), SLOT(handleSpinBoxValueChanged()));
    connect(lat2SpinBox_, SIGNAL(valueChanged(double)), SLOT(handleSpinBoxValueChanged()));
}

FilterControlBase *FreeLineFilterControl::create(QGridLayout *layout, int row, mgp::FreeLineFilter *filter)
{
    QDoubleSpinBox *lon1SpinBox = new QDoubleSpinBox; lon1SpinBox->setDecimals(3);
    QDoubleSpinBox *lat1SpinBox = new QDoubleSpinBox; lat1SpinBox->setDecimals(3);
    QDoubleSpinBox *lon2SpinBox = new QDoubleSpinBox; lon2SpinBox->setDecimals(3);
    QDoubleSpinBox *lat2SpinBox = new QDoubleSpinBox; lat2SpinBox->setDecimals(3);
    FreeLineFilterControl *filterControl = new FreeLineFilterControl(
                filter, new QCheckBox, new QCheckBox, lon1SpinBox, lat1SpinBox, lon2SpinBox, lat2SpinBox);

    QLabel *typeLabel = new QLabel(QString("%1 OF LINE").arg(filter->directionName()));
    typeLabel->setStyleSheet("font-family:mono");
    layout->addWidget(typeLabel, row, 0, Qt::AlignRight);
    layout->addWidget(filterControl->enabledCheckBox_, row, 1, Qt::AlignHCenter);
    layout->addWidget(filterControl->currCheckBox_, row, 2, Qt::AlignHCenter);

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

    return filterControl;
}

void FreeLineFilterControl::update()
{
   lon1SpinBox_->setValue(RAD2DEG(freeLineFilter_->lon1()));
   lat1SpinBox_->setValue(RAD2DEG(freeLineFilter_->lat1()));
   lon2SpinBox_->setValue(RAD2DEG(freeLineFilter_->lon2()));
   lat2SpinBox_->setValue(RAD2DEG(freeLineFilter_->lat2()));
}

QVariant FreeLineFilterControl::value() const
{
    return QLineF(QPointF(lon1SpinBox_->value(), lat1SpinBox_->value()), QPointF(lon2SpinBox_->value(), lat2SpinBox_->value()));
}

bool FreeLineFilterControl::startDragging(const mgp::Point &point)
{
    const double dist1 = mgp::math::Math::distance(point, qMakePair(DEG2RAD(lon1SpinBox_->value()), DEG2RAD(lat1SpinBox_->value())));
    const double dist2 = mgp::math::Math::distance(point, qMakePair(DEG2RAD(lon2SpinBox_->value()), DEG2RAD(lat2SpinBox_->value())));

    firstEndpointDragged_ = (dist1 < dist2);
    dragged_ = true;
    updateDragging(point);
    return true;
}

void FreeLineFilterControl::updateDragging(const mgp::Point &point)
{
    Q_ASSERT(dragged_);

    const double lon = point.first;
    const double lat = point.second;

    if (firstEndpointDragged_) {
        lon1SpinBox_->setValue(RAD2DEG(lon));
        lat1SpinBox_->setValue(RAD2DEG(lat));
        freeLineFilter_->setPoint1(qMakePair(lon, lat));
    } else {
        lon2SpinBox_->setValue(RAD2DEG(lon));
        lat2SpinBox_->setValue(RAD2DEG(lat));
        freeLineFilter_->setPoint2(qMakePair(lon, lat));
    }
}

void FreeLineFilterControl::handleSpinBoxValueChanged()
{
    const double lon1 = DEG2RAD(lon1SpinBox_->value());
    const double lat1 = DEG2RAD(lat1SpinBox_->value());
    const double lon2 = DEG2RAD(lon2SpinBox_->value());
    const double lat2 = DEG2RAD(lat2SpinBox_->value());
    freeLineFilter_->setLine(qMakePair(lon1, lat1), qMakePair(lon2, lat2));
    ControlPanel::instance().updateGLWidget();
}

BasePolygon::BasePolygon(Type type, const mgp::Polygon &points)
    : type_(type)
    , polygon_(points)
{
}

static mgp::Polygon createENORFIR()
{
    mgp::Polygon points = mgp::Polygon(new QVector<mgp::Point>);

    const int npoints = sizeof(enor_fir) / sizeof(float) / 2;
    for (int i = 0; i < npoints; ++i) {
        const double lon = DEG2RAD(enor_fir[2 * i + 1]);
        const double lat = DEG2RAD(enor_fir[2 * i]);
        points->append(qMakePair(lon, lat));
    }
    return points;
}

BasePolygon *BasePolygon::create(Type type)
{
    if (type == None) {
        return new BasePolygon(None);

    } else if (type == Custom) {
        mgp::Polygon points = mgp::Polygon(new QVector<mgp::Point>);
        points->append(qMakePair(DEG2RAD(7), DEG2RAD(60)));
        points->append(qMakePair(DEG2RAD(13), DEG2RAD(60)));
        points->append(qMakePair(DEG2RAD(10), DEG2RAD(65)));
        return new BasePolygon(Custom, points);

    } else if (type == ENOR_FIR) {
        return new BasePolygon(ENOR_FIR, createENORFIR());

    } else {
        return new BasePolygon(type); // for now
    }
}

ResultPolygonsExportPanel::ResultPolygonsExportPanel()
{
    setWindowTitle("Result Polygon(s)");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    textEdit_ = new QTextEdit;
    textEdit_->setReadOnly(true);
    mainLayout->addWidget(textEdit_);

    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(bbox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(bbox);
}

void ResultPolygonsExportPanel::setPolygons(const mgp::Polygons &polygons)
{
    QString html;

    html += "<html>";
    html += "<head>";
    html += "<style type=\"text/css\">";
    html += "table { border:1px; border-collapse:collapse }";
    html += "th { padding-top:0px; padding-bottom:0px; margin-top:0px; margin-bottom:0px; border:1px; padding-left:3px; padding-right:3px }";
    html += "tr { padding-top:0px; padding-bottom:0px; margin-top:0px; margin-bottom:0px; border:1px }";
    html += "td { padding-top:0px; padding-bottom:0px; margin-top:0px; margin-bottom:0px; border:1px; padding-left:3px; padding-right:3px }";
    html += "</style>";
    html += "</head><body>";

    if (polygons->isEmpty())
        html += "no result polygons";

    for (int i = 0; i < polygons->size(); ++i) {
        if (i > 0)
            html += "<br/><br/>";
        const mgp::Polygon polygon = polygons->at(i);
        html += QString("<span style='font-size:large; font-weight:bold'>Polygon %1:%2 (%3 vertices):</span>")
                .arg(i + 1).arg(polygons->size()).arg(polygon->size());
        //html += QString("%1 vertices (skipped)<br/>").arg(polygon->size());
        html += "<table><tr><td></td><td>Longitude</td><td>Latitude</td></tr>";
        for (int j = 0; j < polygon->size(); ++j) {
            const mgp::Point point = polygon->at(j);
            html += QString("<tr><td align=right>%1</td><td align=right>%2</td><td align=right>%3</td></tr>")
                    .arg(j + 1)
                    .arg(RAD2DEG(point.first),  7, 'f', 2)
                    .arg(RAD2DEG(point.second), 6, 'f', 2);
        }
        html += "</table></br>";
    }

    html += "</body></html>";

    textEdit_->setHtml(html);
}

ControlPanel &ControlPanel::instance()
{
    static ControlPanel cp;
    return cp;
}

void ControlPanel::open()
{
    setVisible(true);
    raise();
}

ControlPanel::ControlPanel()
    : bsSlider_(0)
    , basePolygonComboBox_(0)
    , basePolygonLinesVisibleCheckBox_(0)
    , basePolygonPointsVisibleCheckBox_(0)
    , basePolygonIntersectionsVisibleCheckBox_(0)
    , customBasePolygonEditableOnSphereCheckBox_(0)
    , filtersEditableOnSphereCheckBox_(0)
    , filterLinesVisibleCheckBox_(0)
    , filterPointsVisibleCheckBox_(0)
    , resultPolygonsLinesVisibleCheckBox_(0)
    , resultPolygonsPointsVisibleCheckBox_(0)
    , filterTabWidget_(0)
{
}

static QGridLayout *createFilterLayout()
{
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("Type"), 0, 0, Qt::AlignHCenter);
    layout->addWidget(new QLabel("Enabled"), 0, 1, Qt::AlignHCenter);
    layout->addWidget(new QLabel("Current"), 0, 2, Qt::AlignHCenter);
    layout->addWidget(new QLabel("Value"), 0, 3, 1, 2, Qt::AlignLeft);
    for (int i = 0; i < 4; ++i)
        layout->itemAtPosition(0, i)->widget()->setStyleSheet("font-weight:bold");
    return layout;
}

void ControlPanel::initialize()
{
    setWindowTitle("Control Panel");
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // --- BEGIN general section -------------------------------------------
    QGroupBox *generalGroupBox = new QGroupBox("General");
    QVBoxLayout *generalLayout = new QVBoxLayout;
    generalGroupBox->setLayout(generalLayout);
    mainLayout->addWidget(generalGroupBox);

    // ball size
    QHBoxLayout *bsLayout = new QHBoxLayout;
    bsLayout->addWidget(new QLabel("Ball size:"));
    bsSlider_ = new QSlider(Qt::Horizontal);
    bsSlider_->setValue(bsSlider_->minimum() + 0.5 * (bsSlider_->maximum() - bsSlider_->minimum()));
    connect(bsSlider_, SIGNAL(valueChanged(int)), SLOT(updateGLWidget()));
    bsLayout->addWidget(bsSlider_);
    generalLayout->addLayout(bsLayout);

    // coast lines on/off ... TBD
    // result polygon on/off ... TBD

    // --- END general section -------------------------------------------


    // --- BEGIN base polygon section -------------------------------------------
    QGroupBox *basePolygonGroupBox = new QGroupBox("Base Polygon");
    QVBoxLayout *basePolygonLayout = new QVBoxLayout;
    basePolygonGroupBox->setLayout(basePolygonLayout);
    mainLayout->addWidget(basePolygonGroupBox);

    QHBoxLayout *basePolygonLayout1 = new QHBoxLayout;
    basePolygonLayout->addLayout(basePolygonLayout1);

    basePolygonLinesVisibleCheckBox_ = new QCheckBox("Lines");
    basePolygonLinesVisibleCheckBox_->setChecked(true);
    connect(basePolygonLinesVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    basePolygonLayout1->addWidget(basePolygonLinesVisibleCheckBox_);

    basePolygonPointsVisibleCheckBox_ = new QCheckBox("Points");
    basePolygonPointsVisibleCheckBox_->setChecked(false);
    connect(basePolygonPointsVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    basePolygonLayout1->addWidget(basePolygonPointsVisibleCheckBox_);

    basePolygonIntersectionsVisibleCheckBox_ = new QCheckBox("Intersections");
    basePolygonIntersectionsVisibleCheckBox_->setChecked(false);
    connect(basePolygonIntersectionsVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    basePolygonLayout1->addWidget(basePolygonIntersectionsVisibleCheckBox_);

    basePolygonLayout1->addStretch(1);

    QHBoxLayout *basePolygonLayout2 = new QHBoxLayout;
    basePolygonLayout->addLayout(basePolygonLayout2);

    basePolygonComboBox_ = new QComboBox;
    basePolygonComboBox_->addItem("Custom", BasePolygon::Custom);
    basePolygonComboBox_->addItem("ENOR FIR", BasePolygon::ENOR_FIR);
    basePolygonComboBox_->addItem("XXXX FIR", BasePolygon::XXXX_FIR);
    basePolygonComboBox_->addItem("YYYY FIR", BasePolygon::YYYY_FIR);
    basePolygonComboBox_->addItem("ZZZZ FIR", BasePolygon::ZZZZ_FIR);
    basePolygonComboBox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    connect(basePolygonComboBox_, SIGNAL(currentIndexChanged(int)), SLOT(basePolygonTypeChanged()));
    basePolygonLayout2->addWidget(basePolygonComboBox_);

    customBasePolygonEditableOnSphereCheckBox_ = new QCheckBox("Editable on earth sphere");
    connect(customBasePolygonEditableOnSphereCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    basePolygonLayout2->addWidget(customBasePolygonEditableOnSphereCheckBox_);

    basePolygonLayout2->addStretch(1);

    //basePolygons_.insert(BasePolygon::None, BasePolygon::create(BasePolygon::None)); // ### necessary?
    basePolygons_.insert(BasePolygon::Custom, BasePolygon::create(BasePolygon::Custom));
    basePolygons_.insert(BasePolygon::ENOR_FIR, BasePolygon::create(BasePolygon::ENOR_FIR));
    basePolygons_.insert(BasePolygon::XXXX_FIR, BasePolygon::create(BasePolygon::XXXX_FIR));
    basePolygons_.insert(BasePolygon::YYYY_FIR, BasePolygon::create(BasePolygon::YYYY_FIR));
    basePolygons_.insert(BasePolygon::ZZZZ_FIR, BasePolygon::create(BasePolygon::ZZZZ_FIR));

    // --- END base polygon section -------------------------------------------


    // --- BEGIN filter section -------------------------------------------
    QGroupBox *filterGroupBox = new QGroupBox("Filters");
    QVBoxLayout *filterLayout = new QVBoxLayout;
    filterGroupBox->setLayout(filterLayout);
    mainLayout->addWidget(filterGroupBox);

    // header
//    filterLayout->addWidget(new QLabel("Filters:"), 0, 0, 1, 5);
//    filterLayout->itemAtPosition(0, 0)->widget()->setStyleSheet("font-weight:bold; font-size:16px");

    QHBoxLayout *filterLayout2 = new QHBoxLayout;
    filterLayout->addLayout(filterLayout2);

    filtersEditableOnSphereCheckBox_ = new QCheckBox("Editable on earth sphere");
    filterLayout2->addWidget(filtersEditableOnSphereCheckBox_);

    filterLinesVisibleCheckBox_ = new QCheckBox("Lines");
    filterLinesVisibleCheckBox_->setChecked(true);
    connect(filterLinesVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    filterLayout2->addWidget(filterLinesVisibleCheckBox_);

    filterPointsVisibleCheckBox_ = new QCheckBox("Points");
    filterPointsVisibleCheckBox_->setChecked(true);
    connect(filterPointsVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    filterLayout2->addWidget(filterPointsVisibleCheckBox_);

    filterTabWidget_ = new QTabWidget;
    filterLayout->addWidget(filterTabWidget_);

    // page for WI filter
    {
        QWidget *filterPage = new QWidget;
        QGridLayout *filterLayout = createFilterLayout();
        filterPage->setLayout(filterLayout);

        // default value arbitrarily chosen for now
        mgp::Polygon wiPoints (new QVector<mgp::Point>);
        wiPoints->append(qMakePair(0.17, 0.95));
        wiPoints->append(qMakePair(0.3, 1.02));
        wiPoints->append(qMakePair(0.25, 1.08));
        wiPoints->append(qMakePair(0.18, 1.08));
        wiFilter_ = QSharedPointer<mgp::WithinFilter>(new mgp::WithinFilter(wiPoints));
        filterControls_.insert(mgp::FilterBase::WI, WithinFilterControl::create(filterLayout, 1, wiFilter_.data()));
        filterLayout->setRowStretch(2, 1);

        const QString baseText("WI");
        filterTabWidget_->addTab(filterPage, baseText);

        QList<mgp::FilterBase::Type> filterTypes;
        filterTypes.append(mgp::FilterBase::WI);
        filterTabInfos_.append(FilterTabInfo(filterPage, baseText, filterTypes));
    }

    // page for '{E|W|N|S} OF' filters
    {
        QWidget *filterPage = new QWidget;
        QGridLayout *filterLayout = createFilterLayout();
        filterPage->setLayout(filterLayout);

        // default values arbitrarily chosen for now
        filterControls_.insert(mgp::FilterBase::E_OF, LonOrLatFilterControl::create(filterLayout, 1, new mgp::EOfFilter(DEG2RAD(4))));
        filterControls_.insert(mgp::FilterBase::W_OF, LonOrLatFilterControl::create(filterLayout, 2, new mgp::WOfFilter(DEG2RAD(12))));
        filterControls_.insert(mgp::FilterBase::N_OF, LonOrLatFilterControl::create(filterLayout, 3, new mgp::NOfFilter(DEG2RAD(58))));
        filterControls_.insert(mgp::FilterBase::S_OF, LonOrLatFilterControl::create(filterLayout, 4, new mgp::SOfFilter(DEG2RAD(66))));
        filterLayout->setRowStretch(5, 1);

        const QString baseText("{E|W|N|S} OF");
        filterTabWidget_->addTab(filterPage, baseText);

        QList<mgp::FilterBase::Type> filterTypes;
        filterTypes.append(mgp::FilterBase::E_OF);
        filterTypes.append(mgp::FilterBase::W_OF);
        filterTypes.append(mgp::FilterBase::N_OF);
        filterTypes.append(mgp::FilterBase::S_OF);
        filterTabInfos_.append(FilterTabInfo(filterPage, baseText, filterTypes));
    }

    // page for '{E|W|N|S} OF LINE' filters
    {
        QWidget *filterPage = new QWidget;
        QGridLayout *filterLayout = createFilterLayout();
        filterPage->setLayout(filterLayout);

        // default values arbitrarily chosen for now
        filterControls_.insert(mgp::FilterBase::E_OF_LINE, FreeLineFilterControl::create(
                                   filterLayout, 1, new mgp::EOfLineFilter(
                                       qMakePair(qMakePair(DEG2RAD(-6), DEG2RAD(65)), qMakePair(DEG2RAD(-5), DEG2RAD(55))))));
        filterControls_.insert(mgp::FilterBase::W_OF_LINE, FreeLineFilterControl::create(
                                   filterLayout, 2, new mgp::WOfLineFilter(
                                       qMakePair(qMakePair(DEG2RAD(18), DEG2RAD(53)), qMakePair(DEG2RAD(20), DEG2RAD(64))))));
        filterControls_.insert(mgp::FilterBase::N_OF_LINE, FreeLineFilterControl::create(
                                   filterLayout, 3, new mgp::NOfLineFilter(
                                       qMakePair(qMakePair(DEG2RAD(-7), DEG2RAD(54)), qMakePair(DEG2RAD(17), DEG2RAD(55))))));
        filterControls_.insert(mgp::FilterBase::S_OF_LINE, FreeLineFilterControl::create(
                                   filterLayout, 4, new mgp::SOfLineFilter(
                                       qMakePair(qMakePair(DEG2RAD(-7), DEG2RAD(67)), qMakePair(DEG2RAD(19), DEG2RAD(63))))));
        filterLayout->setRowStretch(5, 1);

        const QString baseText("{E|W|N|S} OF LINE");
        filterTabWidget_->addTab(filterPage, baseText);

        QList<mgp::FilterBase::Type> filterTypes;
        filterTypes.append(mgp::FilterBase::E_OF_LINE);
        filterTypes.append(mgp::FilterBase::W_OF_LINE);
        filterTypes.append(mgp::FilterBase::N_OF_LINE);
        filterTypes.append(mgp::FilterBase::S_OF_LINE);
        filterTabInfos_.append(FilterTabInfo(filterPage, baseText, filterTypes));
    }

    // page for '{NE|NW|SE|SW} OF LINE' filters
    {
        QWidget *filterPage = new QWidget;
        QGridLayout *filterLayout = createFilterLayout();
        filterPage->setLayout(filterLayout);

        // default values arbitrarily chosen for now
        filterControls_.insert(mgp::FilterBase::NE_OF_LINE, FreeLineFilterControl::create(
                                   filterLayout, 1, new mgp::NEOfLineFilter(
                                       qMakePair(qMakePair(DEG2RAD(-4), DEG2RAD(65)), qMakePair(DEG2RAD(14), DEG2RAD(55))))));
        filterControls_.insert(mgp::FilterBase::NW_OF_LINE, FreeLineFilterControl::create(
                                   filterLayout, 2, new mgp::NWOfLineFilter(
                                       qMakePair(qMakePair(DEG2RAD(5), DEG2RAD(53)), qMakePair(DEG2RAD(25), DEG2RAD(64))))));
        filterControls_.insert(mgp::FilterBase::SE_OF_LINE, FreeLineFilterControl::create(
                                   filterLayout, 3, new mgp::SEOfLineFilter(
                                       qMakePair(qMakePair(DEG2RAD(-1), DEG2RAD(54)), qMakePair(DEG2RAD(17), DEG2RAD(67))))));
        filterControls_.insert(mgp::FilterBase::SW_OF_LINE, FreeLineFilterControl::create(
                                   filterLayout, 4, new mgp::SWOfLineFilter(
                                       qMakePair(qMakePair(DEG2RAD(-1), DEG2RAD(67)), qMakePair(DEG2RAD(19), DEG2RAD(57))))));
        filterLayout->setRowStretch(5, 1);

        const QString baseText("{NE|NW|SE|SW} OF LINE");
        filterTabWidget_->addTab(filterPage, baseText);

        QList<mgp::FilterBase::Type> filterTypes;
        filterTypes.append(mgp::FilterBase::NE_OF_LINE);
        filterTypes.append(mgp::FilterBase::NW_OF_LINE);
        filterTypes.append(mgp::FilterBase::SE_OF_LINE);
        filterTypes.append(mgp::FilterBase::SW_OF_LINE);
        filterTabInfos_.append(FilterTabInfo(filterPage, baseText, filterTypes));
    }

    // ensure exclusive/radio behavior for the 'current' state
    QButtonGroup *currBtnGroup = new QButtonGroup;
    currBtnGroup->setExclusive(true);
    foreach (FilterControlBase *filter, filterControls_)
        currBtnGroup->addButton(filter->currCheckBox_);

    const mgp::FilterBase::Type initCurrType = mgp::FilterBase::E_OF;
    filterControls_.value(initCurrType)->currCheckBox_->blockSignals(true);
    filterControls_.value(initCurrType)->currCheckBox_->setChecked(true);
    filterControls_.value(initCurrType)->currCheckBox_->blockSignals(false);

    // initialize tab texts
    updateFilterTabTexts();

    // --- END filter section -------------------------------------------


    // --- BEGIN result polygons section -------------------------------------------
    resultPolygonsGroupBox_ = new QGroupBox;
    updateResultPolygonsGroupBoxTitle(-1);
    QVBoxLayout *resultPolygonsLayout = new QVBoxLayout;
    resultPolygonsGroupBox_->setLayout(resultPolygonsLayout);
    mainLayout->addWidget(resultPolygonsGroupBox_);

    QHBoxLayout *resultPolygonsLayout2 = new QHBoxLayout;
    resultPolygonsLayout->addLayout(resultPolygonsLayout2);

    resultPolygonsLinesVisibleCheckBox_ = new QCheckBox("Lines");
    connect(resultPolygonsLinesVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    resultPolygonsLayout2->addWidget(resultPolygonsLinesVisibleCheckBox_);

    resultPolygonsPointsVisibleCheckBox_ = new QCheckBox("Points");
    connect(resultPolygonsPointsVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    resultPolygonsLayout2->addWidget(resultPolygonsPointsVisibleCheckBox_);

    resultPolygonsLayout2->addStretch(1);

    QPushButton *exportButton = new QPushButton("Export");
    connect(exportButton, SIGNAL(clicked()), SLOT(exportResultPolygons()));
    resultPolygonsLayout2->addWidget(exportButton);

    resPolysExportPanel_ = new ResultPolygonsExportPanel;

    // --- END result polygons section -------------------------------------------


    // --- BEGIN SIGMET/AIRMET area expression section -------------------------------------------
    QGroupBox *xmetExprGroupBox = new QGroupBox("SIGMET/AIRMET Area Expression");
    QVBoxLayout *xmetExprLayout = new QVBoxLayout;
    xmetExprGroupBox->setLayout(xmetExprLayout);
    mainLayout->addWidget(xmetExprGroupBox);

    xmetExprEdit_ = new TextEdit;
    xmetExprLayout->addWidget(xmetExprEdit_);
    connect(xmetExprEdit_, SIGNAL(textChanged()), SLOT(handleXmetExprChanged()));

    QHBoxLayout *xmetExprLayout2 = new QHBoxLayout;
    xmetExprLayout->addLayout(xmetExprLayout2);

    QPushButton *setXmetExprFromFiltersButton = new QPushButton("Set expression from filters");
    setXmetExprFromFiltersButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    connect(setXmetExprFromFiltersButton, SIGNAL(clicked()), SLOT(setXmetExprFromFilters()));
    xmetExprLayout2->addWidget(setXmetExprFromFiltersButton);

    xmetExprLayout2->addStretch(1);

    setFiltersFromXmetExprButtonText_ = "Set filters from expression";
    setFiltersFromXmetExprButton_ = new QPushButton(setFiltersFromXmetExprButtonText_);
    setFiltersFromXmetExprButton_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    connect(setFiltersFromXmetExprButton_, SIGNAL(clicked()), SLOT(setFiltersFromXmetExpr()));
    xmetExprLayout2->addWidget(setFiltersFromXmetExprButton_);

    autoSetFiltersCheckBox_ = new QCheckBox("Auto");
    connect(autoSetFiltersCheckBox_, SIGNAL(toggled(bool)), setFiltersFromXmetExprButton_, SLOT(setDisabled(bool)));
    connect(autoSetFiltersCheckBox_, SIGNAL(toggled(bool)), SLOT(setFiltersFromXmetExpr()));
    xmetExprLayout2->addWidget(autoSetFiltersCheckBox_);

    // --- END SIGMET/AIRMET area expression section -------------------------------------------


    // --- BEGIN bottom section -----------------------------------------------
    QFrame *botPanel = new QFrame;
    botPanel->setLayout(new QHBoxLayout);
    mainLayout->addWidget(botPanel);

    qobject_cast<QHBoxLayout *>(botPanel->layout())->addStretch(1);

    QPushButton *closeButton = new QPushButton("Close");
    connect(closeButton, SIGNAL(clicked()), SLOT(close()));
    botPanel->layout()->addWidget(closeButton);
    // --- END bottom section -----------------------------------------------
}

void ControlPanel::keyPressEvent(QKeyEvent *event)
{
    MainWindow::instance().handleKeyPressEvent(event);
}

bool ControlPanel::isEnabled(mgp::FilterBase::Type type) const
{
    return filterControls_.contains(type) ? filterControls_.value(type)->enabledCheckBox_->isChecked() : false;
}

bool ControlPanel::isCurrent(mgp::FilterBase::Type type) const
{
    return filterControls_.contains(type) ? filterControls_.value(type)->currCheckBox_->isChecked() : false;
}

bool ControlPanel::isValid(mgp::FilterBase::Type type) const
{
    return filterControls_.contains(type) ? filterControls_.value(type)->filter()->isValid() : false;
}

mgp::Polygon ControlPanel::WIFilterPolygon() const
{
    return wiFilter_->polygon();
}

QVariant ControlPanel::value(mgp::FilterBase::Type type) const
{
    return filterControls_.value(type)->value();
}

bool ControlPanel::rejectedByAnyFilter(const mgp::Point &point) const
{
    foreach (FilterControlBase *filterControl, filterControls_) {
        if (filterControl->enabledCheckBox_->isChecked() && filterControl->filter()->rejected(point))
            return true;
    }
    return false;
}

// Returns all intersection points between enabled filters and a polygon.
QVector<mgp::Point> ControlPanel::filterIntersections(const mgp::Polygon &inPoly) const
{
    QVector<mgp::Point> points;
    foreach (FilterControlBase *filterControl, filterControls_) {
        if (filterControl->enabledCheckBox_->isChecked())
            points += filterControl->filter()->intersections(inPoly);
    }
    return points;
}

bool ControlPanel::filtersEditableOnSphere() const
{
    return filtersEditableOnSphereCheckBox_->isChecked();
}

void ControlPanel::toggleFiltersEditableOnSphere()
{
    filtersEditableOnSphereCheckBox_->toggle();
}

bool ControlPanel::filterLinesVisible() const
{
    return filterLinesVisibleCheckBox_->isChecked();
}

bool ControlPanel::filterPointsVisible() const
{
    return filterPointsVisibleCheckBox_->isChecked();
}

// If we're in 'filters editable on sphere' mode and the current filter is enabled, this function initializes
// dragging of that filter at the given pos.
bool ControlPanel::startFilterDragging(const mgp::Point &point) const
{
    if (!filtersEditableOnSphereCheckBox_->isChecked())
        return false; // wrong mode (hm ... should this be a Q_ASSERT() instead?)

    // ensure no filter is currently considered as being dragged
    foreach (FilterControlBase *filter, filterControls_)
        filter->dragged_ = false;

    // apply the operation to the current filter if it is enabled
    foreach (FilterControlBase *filter, filterControls_) {
        if (filter->currCheckBox_->isChecked())
            return (filter->enabledCheckBox_->isChecked() && filter->startDragging(point));
    }

    return false; // no match
}

// If there is a draggable filter, tell this filter to update its draggable control point with this pos, and update the GLWidget.
void ControlPanel::updateFilterDragging(const mgp::Point &point)
{
    foreach (FilterControlBase *filter, filterControls_) {
        if (filter->dragged_) {
            filter->updateDragging(point);
            return;
        }
    }
}

BasePolygon::Type ControlPanel::currentBasePolygonType() const
{
    if (!basePolygonComboBox_)
        return BasePolygon::None;
    return static_cast<BasePolygon::Type>(basePolygonComboBox_->itemData(basePolygonComboBox_->currentIndex()).toInt());
}

bool ControlPanel::basePolygonLinesVisible() const
{
    return basePolygonLinesVisibleCheckBox_->isChecked();
}

bool ControlPanel::basePolygonPointsVisible() const
{
    return basePolygonPointsVisibleCheckBox_->isChecked();
}

bool ControlPanel::basePolygonIntersectionsVisible() const
{
    return basePolygonIntersectionsVisibleCheckBox_->isChecked();
}

mgp::Polygon ControlPanel::currentBasePolygon() const
{
    if (!basePolygonComboBox_)
        return mgp::Polygon();

    const BasePolygon::Type currType = static_cast<BasePolygon::Type>(basePolygonComboBox_->itemData(basePolygonComboBox_->currentIndex()).toInt());
    Q_ASSERT(basePolygons_.contains(currType));
    return basePolygons_.value(currType)->polygon_;
}

static int currentPolygonPoint(const mgp::Polygon &polygon, const mgp::Point &point, double tolerance)
{
    for (int i = 0; i < polygon->size(); ++i) {
        const double dist = mgp::math::Math::distance(point, polygon->at(i));
        if (dist < tolerance)
            return i;
    }
    return -1;
}

int ControlPanel::currentWIFilterPoint(const mgp::Point &point, double tolerance)
{
    return currentPolygonPoint(wiFilter_->polygon(), point, tolerance);
}

int ControlPanel::currentCustomBasePolygonPoint(const mgp::Point &point, double tolerance)
{
    if (currentBasePolygonType() != BasePolygon::Custom)
        return -1;    
    return currentPolygonPoint(basePolygons_.value(BasePolygon::Custom)->polygon_, point, tolerance);
}

bool ControlPanel::customBasePolygonEditableOnSphere() const
{
    return customBasePolygonEditableOnSphereCheckBox_->isChecked();
}

void ControlPanel::updatePolygonPointDragging(const mgp::Polygon &points, int index, const mgp::Point &point)
{
    Q_ASSERT((index >= 0) && (index < points->size()));
    (*points)[index] = point;
    updateGLWidget();
}

void ControlPanel::updateWIFilterPointDragging(int index, const mgp::Point &point)
{
    updatePolygonPointDragging(wiFilter_->polygon(), index, point);
}

void ControlPanel::updateCustomBasePolygonPointDragging(int index, const mgp::Point &point)
{
    updatePolygonPointDragging(basePolygons_.value(BasePolygon::Custom)->polygon_, index, point);
}

void ControlPanel::addPointToPolygon(const mgp::Polygon &polygon, int index)
{
    Q_ASSERT((index >= 0) && (index < polygon->size()));

    const double lon1 = polygon->at(index).first;
    const double lat1 = polygon->at(index).second;
    const int index2 = (index - 1 + polygon->size()) % polygon->size();
    const double lon2 = polygon->at(index2).first;
    const double lat2 = polygon->at(index2).second;
    polygon->insert(index, qMakePair(0.5 * (lon1 + lon2), 0.5 * (lat1 + lat2)));
    MainWindow::instance().glWidget()->updateCurrCustomBasePolygonPoint();
    updateGLWidget();
}

void ControlPanel::addPointToWIFilter(int index)
{
    addPointToPolygon(wiFilter_->polygon(), index);
    MainWindow::instance().glWidget()->updateWIFilterPoint();
    updateGLWidget();
}

void ControlPanel::addPointToCustomBasePolygon(int index)
{
    addPointToPolygon(basePolygons_.value(BasePolygon::Custom)->polygon_, index);
    MainWindow::instance().glWidget()->updateCurrCustomBasePolygonPoint();
    updateGLWidget();
}

void ControlPanel::removePointFromPolygon(const mgp::Polygon &polygon, int index)
{
    Q_ASSERT((index >= 0) && (index < polygon->size()));

    if (polygon->size() <= 3)
        return; // we need to have at least a triangle!

    polygon->remove(index);
    MainWindow::instance().glWidget()->updateCurrCustomBasePolygonPoint();
    updateGLWidget();
}

void ControlPanel::removePointFromWIFilter(int index)
{
    removePointFromPolygon(wiFilter_->polygon(), index);
}

void ControlPanel::removePointFromCustomBasePolygon(int index)
{
    removePointFromPolygon(basePolygons_.value(BasePolygon::Custom)->polygon_, index);
}

// Returns true iff the current base polygon is oriented clockwise.
bool ControlPanel::currentBasePolygonIsClockwise() const
{
    return mgp::math::isClockwise(currentBasePolygon());
}

// Returns true iff the given point is considered inside the current base polygon.
bool ControlPanel::withinCurrentBasePolygon(const mgp::Point &point) const
{
    return mgp::math::pointInPolygon(point, currentBasePolygon());
}

bool ControlPanel::resultPolygonsLinesVisible() const
{
    return resultPolygonsLinesVisibleCheckBox_->isChecked();
}

bool ControlPanel::resultPolygonsPointsVisible() const
{
    return resultPolygonsPointsVisibleCheckBox_->isChecked();
}

// Returns polygons resulting from applying the sequence of enabled and valid filters to the base polygon.
mgp::Polygons ControlPanel::resultPolygons() const
{
    return mgp::applyFilters(currentBasePolygon(), enabledAndValidFilters());
}

void ControlPanel::updateResultPolygonsGroupBoxTitle(int n)
{
    resultPolygonsGroupBox_->setTitle(QString("Result Polygons%1").arg(n < 0 ? QString() : QString(" (%1)").arg(n)));
}

float ControlPanel::ballSizeFrac()
{
    return bsSlider_ ? (float(bsSlider_->value() - bsSlider_->minimum()) / (bsSlider_->maximum() - bsSlider_->minimum())) : 0.0;
}

FilterControlBase *ControlPanel::currentFilter() const
{
    foreach (FilterControlBase *filterControl, filterControls_)
        if (filterControl->currCheckBox_->isChecked())
            return filterControl;
    Q_ASSERT(false);
    return 0;
}

// Returns the sequence of enabled and valid filters.
mgp::Filters ControlPanel::enabledAndValidFilters() const
{
    mgp::Filters filters(new QList<mgp::Filter>);
    foreach (FilterControlBase *filterControl, filterControls_) {
        if (filterControl->enabledCheckBox_->isChecked() && filterControl->filter()->isValid())
            filters->append(filterControl->filter());
    }
    return filters;
}

void ControlPanel::updateGLWidget()
{
    MainWindow::instance().glWidget()->updateGL();
}

void ControlPanel::close()
{
    setVisible(false);
}

void ControlPanel::updateFilterTabTexts()
{
    foreach (FilterTabInfo fti, filterTabInfos_) {
        const int tabIndex = filterTabWidget_->indexOf(fti.page_);
        Q_ASSERT(tabIndex >= 0);

        int enabledCount = 0;
        foreach (mgp::FilterBase::Type type, fti.filterTypes_) {
            if (filterControls_.value(type)->enabledCheckBox_->isChecked())
                enabledCount++;
        }

        filterTabWidget_->setTabText(tabIndex, QString("[%1] %2").arg(enabledCount).arg(fti.baseText_));
    }
}

void ControlPanel::basePolygonTypeChanged()
{
    customBasePolygonEditableOnSphereCheckBox_->setVisible(currentBasePolygonType() == BasePolygon::Custom);
    updateGLWidget();
}

void ControlPanel::exportResultPolygons()
{
    resPolysExportPanel_->setPolygons(resultPolygons());
    resPolysExportPanel_->exec();
}

// Sets a canonical SIGMET/AIRMET expression from the currently enabled and valid filters.
void ControlPanel::setXmetExprFromFilters()
{
    xmetExprEdit_->setHtml(mgp::xmetExprFromFilters(enabledAndValidFilters()));
    setFiltersFromXmetExprButton_->setText(setFiltersFromXmetExprButtonText_); // indicate that all changes are updated
}

// Sets the filters from the SIGMET/AIRMET expression if possible.
void ControlPanel::setFiltersFromXmetExpr()
{
    // disable all filters
    foreach (FilterControlBase *filterControl, filterControls_) {
        filterControl->enabledCheckBox_->setChecked(false);
    }

    // parse expression
    QList<QPair<int, int> > matchedRanges;
    QList<QPair<QPair<int, int>, QString> > incompleteRanges;
    const mgp::Filters filters = mgp::filtersFromXmetExpr(xmetExprEdit_->toPlainText(), &matchedRanges, &incompleteRanges);

    // update text edit
    xmetExprEdit_->resetHighlighting();
    for (int i = 0; i < matchedRanges.size(); ++i)
        xmetExprEdit_->addMatchedRange(matchedRanges.at(i));
    for (int i = 0; i < incompleteRanges.size(); ++i)
        xmetExprEdit_->addIncompleteRange(incompleteRanges.at(i).first, incompleteRanges.at(i).second);
    xmetExprEdit_->showHighlighting();

    setFiltersFromXmetExprButton_->setText(setFiltersFromXmetExprButtonText_); // indicate that all changes are updated

    // update filters
    foreach (mgp::Filter filter, *filters) {
        FilterControlBase *filterControl = filterControls_.value(filter->type());
        filterControl->filter()->setFromVariant(filter->toVariant());
        filterControl->update();
        filterControl->enabledCheckBox_->setChecked(true);
    }

    // update GL widget
    if (!matchedRanges.isEmpty())
        updateGLWidget();
}

void ControlPanel::handleXmetExprChanged()
{
    if (autoSetFiltersCheckBox_->isChecked()) {
        // update filters right away
        QTextCursor cursor(xmetExprEdit_->textCursor());
        const int cursorPos = cursor.position();

        xmetExprEdit_->blockSignals(true);
        setFiltersFromXmetExpr();
        xmetExprEdit_->blockSignals(false);

        cursor.setPosition(cursorPos);
        xmetExprEdit_->setTextCursor(cursor);
    } else {
        setFiltersFromXmetExprButton_->setText(setFiltersFromXmetExprButtonText_ + " *"); // indicate that non-updated changes exist
    }
}
