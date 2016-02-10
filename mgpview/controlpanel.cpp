#include "controlpanel.h"
#include "common.h"
#include "mainwindow.h"
#include "glwidget.h"
#include "enor_fir.h"
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
#include <algorithm>
#if 0
#include <boost/geometry.hpp>
#endif

Filter::Filter(Type type, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox)
    : type_(type)
    , enabledCheckBox_(enabledCheckBox)
    , currCheckBox_(currCheckBox)
    , dragged_(false)
{
    connect(enabledCheckBox_, SIGNAL(stateChanged(int)), &ControlPanel::instance(), SLOT(updateGLWidget()));
    connect(currCheckBox_, SIGNAL(stateChanged(int)), &ControlPanel::instance(), SLOT(updateGLWidget()));
}

QString Filter::typeName(Type type)
{
    switch (type) {
    case  None: return  "none";
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

bool Filter::rejected(const QPair<double, double> &point) const
{
    return rejected(point.first, point.second);
}

// Applies the filter to a polygon and returns the result as zero or more polygons within the original area.
PointVectors Filter::apply(const PointVector &inPoly) const
{
    PointVectors outPolys = PointVectors(new QVector<PointVector>());

    // get rejection status for all points
    const int n = inPoly->size();
    QBitArray rej(n);
    for (int i = 0; i < n; ++i)
        rej[i] = rejected(inPoly->at(i).first, inPoly->at(i).second);

    if (rej.count(true) == n) {
        // all points rejected, so return an empty list
        return outPolys;
    } else if (rej.count(true) == 0) {
        // all points accepted, so return a list with one item: a deep copy (although implicitly shared for efficiency) of the input polygon
        PointVector inPolyCopy(new QVector<QPair<double, double> >(*inPoly.data()));
        outPolys->append(inPolyCopy);
        return outPolys;
    }

    // general case

    // find the first transition from rejected to accepted
    int first; // index of the first point in the first polygon
    for (first = 0; first < n; ++first)
        if (rej[first] && (!rej[(first + 1) % n]))
            break;
    int curr = first;

    while (true) {
        // start new polygon at intersection on (curr, curr + 1)
        const int next = (curr + 1) % n;
        Q_ASSERT(rej[curr] && (!rej[next]));
        PointVector poly(new QVector<QPair<double, double> >());
        QPair<double, double> isctPoint1;
        const bool isct1 = intersects(inPoly->at(curr), inPoly->at(next), &isctPoint1);
        if (isct1) // we need this test for the cases where the intersection is located directly on a point etc.
            poly->append(isctPoint1);

        curr = next; // move to next point after intersection

        // append points to current polygon
        while (true) {
            if (!rej[curr]) {
                poly->append(inPoly->at(curr));
                curr = (curr + 1) % n;
            } else {
                // end the current polygon at intersection on (curr - 1, curr)
                const int prev = (curr - 1 + n) % n;
                Q_ASSERT(!rej[prev] && (rej[curr]));
                QPair<double, double> isctPoint2;
                const bool isct2 = intersects(inPoly->at(prev), inPoly->at(curr), &isctPoint2);
                if (isct2) // we need this test for the cases where the intersection is located directly on a point etc.
                    poly->append(isctPoint2);
                outPolys->append(poly);
                break;
            }
        }

        // find start of next polygon if any
        Q_ASSERT(rej[curr]);
        while ((curr != first) && (rej[curr] && rej[(curr + 1) % n]))
            curr = (curr + 1) % n;

        if (curr == first)
            break; // back to where we started!

        Q_ASSERT(rej[curr] && (!rej[(curr + 1) % n]));
    }

    return outPolys;
}

LonOrLatFilter::LonOrLatFilter(
        Type type, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox, QDoubleSpinBox *valSpinBox, double defaultValue)
    : Filter(type, enabledCheckBox, currCheckBox)
    , valSpinBox_(valSpinBox)
{
    if ((type == E_OF) || (type == W_OF)) {
        valSpinBox_->setMinimum(-180);
        valSpinBox_->setMaximum(180);
    } else {
        valSpinBox_->setMinimum(-90);
        valSpinBox_->setMaximum(90);
    }
    valSpinBox_->setValue(defaultValue);
    connect(valSpinBox_, SIGNAL(valueChanged(double)), &ControlPanel::instance(), SLOT(updateGLWidget()));
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

bool LonOrLatFilter::startDragging(double lon_, double lat_)
{
    const double lon = RAD2DEG(lon_);
    const double lat = RAD2DEG(lat_);

    const double val = ((type_ == W_OF) || (type_ == E_OF)) ? lon : lat;
    valSpinBox_->setValue(val);

    dragged_ = true;
    return true;
}

void LonOrLatFilter::updateDragging(double lon_, double lat_)
{
    Q_ASSERT(dragged_);

    const double lon = RAD2DEG(lon_);
    const double lat = RAD2DEG(lat_);

    const double val = ((type_ == W_OF) || (type_ == E_OF)) ? lon : lat;
    valSpinBox_->setValue(val);
}

bool LonOrLatFilter::isValid() const
{
    return true; // ensured by the QSpinBox
}

bool LonOrLatFilter::rejected(double lon_, double lat_) const
{
    const double lon = RAD2DEG(lon_);
    const double lat = RAD2DEG(lat_);
    const double val = valSpinBox_->value();

    switch (type_) {
    case E_OF: return lon < val;
    case W_OF: return lon > val;
    case N_OF: return lat < val;
    case S_OF: return lat > val;
    default: return true;
    }

    return true;
}

bool LonOrLatFilter::intersects(const QPair<double, double> &p1, const QPair<double, double> &p2, QPair<double, double> *isctPoint) const
{
    if ((type_ == N_OF) || (type_ == S_OF)) {
        return Math::intersectsLatitude(p1, p2, DEG2RAD(valSpinBox_->value()), isctPoint);
    }

    const double lon = DEG2RAD(valSpinBox_->value());
    const double minLat = qMin(p1.second, p2.second);
    const double maxLat = qMax(p1.second, p2.second);
    double lat1;
    double lat2;
    if (minLat > 0 && maxLat > 0) {
        lat1 = -M_PI / 4;
        lat2 = M_PI / 2;
    } else if (minLat < 0 && maxLat < 0) {
        lat1 = -M_PI / 2;
        lat2 = M_PI / 4;
    } else {
        lat1 = 0.99 * -M_PI / 2;
        lat2 = 0.99 * M_PI / 2;
    }
    return Math::greatCircleArcsIntersect(p1, p2, lon, lat1, lon, lat2, isctPoint);
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
    , firstEndpointDragged_(false)
{
    lon1SpinBox_->setMinimum(-180);
    lon1SpinBox_->setMaximum(180);
    lon1SpinBox_->setValue(defaultValue.x1());

    lat1SpinBox_->setMinimum(-90);
    lat1SpinBox_->setMaximum(90);
    lat1SpinBox_->setValue(defaultValue.y1());

    lon2SpinBox_->setMinimum(-180);
    lon2SpinBox_->setMaximum(180);
    lon2SpinBox_->setValue(defaultValue.x2());

    lat2SpinBox_->setMinimum(-90);
    lat2SpinBox_->setMaximum(90);
    lat2SpinBox_->setValue(defaultValue.y2());

    connect(lon1SpinBox_, SIGNAL(valueChanged(double)), &ControlPanel::instance(), SLOT(updateGLWidget()));
    connect(lat1SpinBox_, SIGNAL(valueChanged(double)), &ControlPanel::instance(), SLOT(updateGLWidget()));
    connect(lon2SpinBox_, SIGNAL(valueChanged(double)), &ControlPanel::instance(), SLOT(updateGLWidget()));
    connect(lat2SpinBox_, SIGNAL(valueChanged(double)), &ControlPanel::instance(), SLOT(updateGLWidget()));
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

bool FreeLineFilter::startDragging(double lon_, double lat_)
{
    const double lon = RAD2DEG(lon_);
    const double lat = RAD2DEG(lat_);
    const double dist1 = Math::distance(lon, lat, lon1SpinBox_->value(), lat1SpinBox_->value());
    const double dist2 = Math::distance(lon, lat, lon2SpinBox_->value(), lat2SpinBox_->value());

    firstEndpointDragged_ = (dist1 < dist2);
    dragged_ = true;
    updateDragging(lon_, lat_);
    return true;
}

void FreeLineFilter::updateDragging(double lon_, double lat_)
{
    Q_ASSERT(dragged_);

    const double lon = RAD2DEG(lon_);
    const double lat = RAD2DEG(lat_);

    if (firstEndpointDragged_) {
        lon1SpinBox_->setValue(lon);
        lat1SpinBox_->setValue(lat);
    } else {
        lon2SpinBox_->setValue(lon);
        lat2SpinBox_->setValue(lat);
    }
}

bool FreeLineFilter::isValid() const
{
    return validCombination(lon1SpinBox_->value(), lat1SpinBox_->value(), lon2SpinBox_->value(), lat2SpinBox_->value());
}

bool FreeLineFilter::rejected(double lon, double lat) const
{
    const double lon0 = RAD2DEG(lon);
    const double lat0 = RAD2DEG(lat);
    double lon1 = lon1SpinBox_->value();
    double lat1 = lat1SpinBox_->value();
    double lon2 = lon2SpinBox_->value();
    double lat2 = lat2SpinBox_->value();
    if (
            (((type_ == NE_OF) || (type_ == NW_OF)) && (lon1 > lon2)) ||
            (((type_ == SE_OF) || (type_ == SW_OF)) && (lat1 > lat2))) {
        std::swap(lon1, lon2);
        std::swap(lat1, lat2);
    }

    const double crossDist = Math::crossTrackDistanceToGreatCircle(lon0, lat0, lon1, lat1, lon2, lat2);

    switch (type_) {
    case NE_OF: return crossDist > 0;
    case NW_OF: return crossDist > 0;
    case SE_OF: return crossDist < 0;
    case SW_OF: return crossDist > 0;
    default: return true;
    }

    return true;
}

bool FreeLineFilter::intersects(const QPair<double, double> &p1, const QPair<double, double> &p2, QPair<double, double> *isctPoint) const
{
    const double lon1 = DEG2RAD(lon1SpinBox_->value());
    const double lat1 = DEG2RAD(lat1SpinBox_->value());
    const double lon2 = DEG2RAD(lon2SpinBox_->value());
    const double lat2 = DEG2RAD(lat2SpinBox_->value());
    return Math::greatCircleArcsIntersect(p1, p2, lon1, lat1, lon2, lat2, isctPoint);
}

bool FreeLineFilter::validCombination(double lon1, double lat1, double lon2, double lat2) const
{
    return ((type_ == NW_OF) || (type_ == SE_OF))
            ? (((lon1 < lon2) && (lat1 < lat2)) || ((lon1 > lon2) && (lat1 > lat2)))
            : (((lon1 < lon2) && (lat1 > lat2)) || ((lon1 > lon2) && (lat1 < lat2)));
}

BasePolygon::BasePolygon(Type type, const PointVector &points)
    : type_(type)
    , points_(points)
{
}

static PointVector createENORFIR()
{
    PointVector points = PointVector(new QVector<QPair<double, double> >);

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
        PointVector points = PointVector(new QVector<QPair<double, double> >);
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
    , basePolygonVisibleCheckBox_(0)
    , customBasePolygonEditableOnSphereCheckBox_(0)
    , filtersEditableOnSphereCheckBox_(0)
    , resultPolygonsLinesVisibleCheckBox_(0)
    , resultPolygonsPointsVisibleCheckBox_(0)
{
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

    basePolygonVisibleCheckBox_ = new QCheckBox("Visible");
    basePolygonVisibleCheckBox_->setChecked(true);
    connect(basePolygonVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    basePolygonLayout->addWidget(basePolygonVisibleCheckBox_);

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
    QGridLayout *filterLayout = new QGridLayout;
    filterGroupBox->setLayout(filterLayout);
    mainLayout->addWidget(filterGroupBox);

    // header
//    filterLayout->addWidget(new QLabel("Filters:"), 0, 0, 1, 5);
//    filterLayout->itemAtPosition(0, 0)->widget()->setStyleSheet("font-weight:bold; font-size:16px");

    filtersEditableOnSphereCheckBox_ = new QCheckBox("Editable on earth sphere");
    filterLayout->addWidget(filtersEditableOnSphereCheckBox_, 1, 0, 1, 5, Qt::AlignLeft);

    filterLayout->addWidget(new QLabel("Type"), 2, 0, Qt::AlignHCenter);
    filterLayout->addWidget(new QLabel("Enabled"), 2, 1, Qt::AlignHCenter);
    filterLayout->addWidget(new QLabel("Current"), 2, 2, Qt::AlignHCenter);
    filterLayout->addWidget(new QLabel("Value"), 2, 3, 1, 2, Qt::AlignHCenter);
    for (int i = 0; i < 4; ++i)
        filterLayout->itemAtPosition(2, i)->widget()->setStyleSheet("font-weight:bold");

    // lon|lat filters (default values arbitrarily chosen for now)
    filters_.insert(Filter::E_OF, LonOrLatFilter::create(filterLayout, 3, Filter::E_OF, 4));
    filters_.insert(Filter::W_OF, LonOrLatFilter::create(filterLayout, 4, Filter::W_OF, 12));
    filters_.insert(Filter::N_OF, LonOrLatFilter::create(filterLayout, 5, Filter::N_OF, 58));
    filters_.insert(Filter::S_OF, LonOrLatFilter::create(filterLayout, 6, Filter::S_OF, 66));

    // line filters (default values arbitrarily chosen for now)
    filters_.insert(Filter::NE_OF, FreeLineFilter::create(filterLayout, 7, Filter::NE_OF, QLineF(QPointF(-4, 65), QPointF(14, 55))));
    filters_.insert(Filter::NW_OF, FreeLineFilter::create(filterLayout, 8, Filter::NW_OF, QLineF(QPointF(5, 53), QPointF(25, 64))));
    filters_.insert(Filter::SE_OF, FreeLineFilter::create(filterLayout, 9, Filter::SE_OF, QLineF(QPointF(-1, 54), QPointF(17, 67))));
    filters_.insert(Filter::SW_OF, FreeLineFilter::create(filterLayout, 19, Filter::SW_OF, QLineF(QPointF(-1, 67), QPointF(19, 57))));

    // ensure exclusive/radio behavior for the 'current' state
    QButtonGroup *currBtnGroup = new QButtonGroup;
    currBtnGroup->setExclusive(true);
    foreach (Filter *filter, filters_)
        currBtnGroup->addButton(filter->currCheckBox_);

    const Filter::Type initCurrType = Filter::E_OF;
    filters_.value(initCurrType)->currCheckBox_->blockSignals(true);
    filters_.value(initCurrType)->currCheckBox_->setChecked(true);
    filters_.value(initCurrType)->currCheckBox_->blockSignals(false);

    // --- END filter section -------------------------------------------


    // --- BEGIN result polygons section -------------------------------------------
    QGroupBox *resultPolygonsGroupBox = new QGroupBox("Result Polygons");
    QVBoxLayout *resultPolygonsLayout = new QVBoxLayout;
    resultPolygonsGroupBox->setLayout(resultPolygonsLayout);
    mainLayout->addWidget(resultPolygonsGroupBox);

    QHBoxLayout *resultPolygonsLayout2 = new QHBoxLayout;
    resultPolygonsLayout->addLayout(resultPolygonsLayout2);

    resultPolygonsLinesVisibleCheckBox_ = new QCheckBox("Lines");
    connect(resultPolygonsLinesVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    resultPolygonsLayout2->addWidget(resultPolygonsLinesVisibleCheckBox_);

    resultPolygonsPointsVisibleCheckBox_ = new QCheckBox("Points");
    connect(resultPolygonsPointsVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    resultPolygonsLayout2->addWidget(resultPolygonsPointsVisibleCheckBox_);

    resultPolygonsLayout2->addStretch(1);

    // --- END result polygons section -------------------------------------------


    // --- BEGIN SIGMET/AIRMET expression section -------------------------------------------
    QGroupBox *xmetExprGroupBox = new QGroupBox("SIGMET/AIRMET Expression");
    QVBoxLayout *xmetExprLayout = new QVBoxLayout;
    xmetExprGroupBox->setLayout(xmetExprLayout);
    mainLayout->addWidget(xmetExprGroupBox);

    QTextEdit *xmetExprEdit = new QTextEdit;
    xmetExprLayout->addWidget(xmetExprEdit);

    // --- END SIGMET/AIRMET expression section -------------------------------------------


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

bool ControlPanel::isEnabled(Filter::Type type) const
{
    return filters_.contains(type) ? filters_.value(type)->enabledCheckBox_->isChecked() : false;
}

bool ControlPanel::isCurrent(Filter::Type type) const
{
    return filters_.contains(type) ? filters_.value(type)->currCheckBox_->isChecked() : false;
}

bool ControlPanel::isValid(Filter::Type type) const
{
    return filters_.contains(type) ? filters_.value(type)->isValid() : false;
}

QVariant ControlPanel::value(Filter::Type type) const
{
    return filters_.value(type)->value();
}

bool ControlPanel::rejectedByAnyFilter(double lon, double lat) const
{
    foreach (Filter *filter, filters_) {
        if (filter->enabledCheckBox_->isChecked() && filter->rejected(lon, lat))
            return true;
    }
    return false;
}

// Returns all intersection points between enabled filters and the great circle segment between p1 and p2.
QVector<QPair<double, double> > ControlPanel::filterIntersections(const QPair<double, double> &p1, const QPair<double, double> &p2) const
{
    QVector<QPair<double, double> > points;

    foreach (Filter *filter, filters_) {
        QPair<double, double> point;
        if (filter->enabledCheckBox_->isChecked() && (filter->rejected(p1) != filter->rejected(p2)) && filter->intersects(p1, p2, &point))
            points.append(point);
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

// If we're in 'filters editable on sphere' mode and the current filter is enabled, this function initializes
// dragging of that filter at the given pos.
bool ControlPanel::startFilterDragging(double lon, double lat) const
{
    if (!filtersEditableOnSphereCheckBox_->isChecked())
        return false; // wrong mode (hm ... should this be a Q_ASSERT() instead?)

    // ensure no filter is currently considered as being dragged
    foreach (Filter *filter, filters_)
        filter->dragged_ = false;

    // apply the operation to the current filter if it is enabled
    foreach (Filter *filter, filters_) {
        if (filter->currCheckBox_->isChecked())
            return (filter->enabledCheckBox_->isChecked() && filter->startDragging(lon, lat));
    }

    return false; // no match
}

// If there is a draggable filter, tell this filter to update its draggable control point with this pos, and update the GLWidget.
void ControlPanel::updateFilterDragging(double lon, double lat)
{
    foreach (Filter *filter, filters_) {
        if (filter->dragged_) {
            filter->updateDragging(lon, lat);
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

bool ControlPanel::basePolygonVisible() const
{
    return basePolygonVisibleCheckBox_->isChecked();
}

PointVector ControlPanel::currentBasePolygonPoints() const
{
    if (!basePolygonComboBox_)
        return PointVector();

    const BasePolygon::Type currType = static_cast<BasePolygon::Type>(basePolygonComboBox_->itemData(basePolygonComboBox_->currentIndex()).toInt());
    Q_ASSERT(basePolygons_.contains(currType));
    return basePolygons_.value(currType)->points_;
}

int ControlPanel::currentCustomBasePolygonPoint(double lon, double lat, double tolerance)
{
    if (currentBasePolygonType() != BasePolygon::Custom)
        return -1;

    const PointVector points = basePolygons_.value(BasePolygon::Custom)->points_;
    for (int i = 0; i < points->size(); ++i) {
        const double dist = Math::distance(RAD2DEG(lon), RAD2DEG(lat), RAD2DEG(points->at(i).first), RAD2DEG(points->at(i).second));
        if (dist < tolerance)
            return i;
    }
    return -1;
}

bool ControlPanel::customBasePolygonEditableOnSphere() const
{
    return customBasePolygonEditableOnSphereCheckBox_->isChecked();
}

void ControlPanel::updateCustomBasePolygonPointDragging(int index, double lon, double lat)
{
    PointVector points = basePolygons_.value(BasePolygon::Custom)->points_;
    Q_ASSERT((index >= 0) && (index < points->size()));
    (*points)[index] = qMakePair(lon, lat);
    updateGLWidget();
}

void ControlPanel::addPointToCustomBasePolygon(int index)
{
    PointVector points = basePolygons_.value(BasePolygon::Custom)->points_;
    Q_ASSERT((index >= 0) && (index < points->size()));

    const double lon1 = points->at(index).first;
    const double lat1 = points->at(index).second;
    const int index2 = (index - 1 + points->size()) % points->size();
    const double lon2 = points->at(index2).first;
    const double lat2 = points->at(index2).second;
    points->insert(index, qMakePair(0.5 * (lon1 + lon2), 0.5 * (lat1 + lat2)));
    MainWindow::instance().glWidget()->updateCurrCustomBasePolygonPoint();
    updateGLWidget();
}

void ControlPanel::removePointFromCustomBasePolygon(int index)
{
    PointVector points = basePolygons_.value(BasePolygon::Custom)->points_;
    Q_ASSERT((index >= 0) && (index < points->size()));

    if (points->size() <= 3)
        return; // we need to have at least a triangle!

    points->remove(index);
    MainWindow::instance().glWidget()->updateCurrCustomBasePolygonPoint();
    updateGLWidget();
}

bool ControlPanel::withinCurrentBasePolygon(double lon, double lat) const
{
#if 0
    namespace bg = boost::geometry;
    typedef bg::model::point<double, 2, bg::cs::spherical<bg::radian> > point_t;
    typedef bg::model::polygon<point_t> polygon_t;

    const PointVector points = currentBasePolygonPoints();
    //qDebug() << "points:" << *points;
    polygon_t polygon;
    for (int i = 0; i < points->size(); ++i)
        bg::append(polygon.outer(), point_t(points->at(i).first, points->at(i).second));

    const point_t point(lon, lat);
    //std::cerr << bg::dsv(point) << std::endl;
    //std::cerr << bg::get<0>(point) << ", " << bg::get<1>(point) << std::endl;
    //std::cerr << point.get<0>() << ", " << point.get<1>() << std::endl;

    //return bg::within(point, polygon); APPARENTLY NOT SUPPORTED FOR SPHERICAL COORDINATES!
    return bg::within(point, polygon, bg::strategy::within::winding<point_t, point_t, void>());
#else
    int nisct = 0; // number of intersections

    // define a point that is assumed to be outside the polygon
    // ### FOR NOW:
    const double extLon = 0;
    const double extLat = -M_PI;

    const PointVector points = currentBasePolygonPoints();
    for (int i = 0; i < points->size(); ++i) {
        QPair<double, double> isctPoint; // ### not used!
        if (Math::greatCircleArcsIntersect(
                    points->at(i), points->at((i + 1) % points->size()),
                    lon, lat, extLon, extLat, &isctPoint))
            nisct++;
    }

    return nisct % 2;
#endif
}

bool ControlPanel::resultPolygonsLinesVisible() const
{
    return resultPolygonsLinesVisibleCheckBox_->isChecked();
}

bool ControlPanel::resultPolygonsPointsVisible() const
{
    return resultPolygonsPointsVisibleCheckBox_->isChecked();
}

PointVectors ControlPanel::resultPolygons() const
{
    PointVectors resultPolys(new QVector<PointVector>());

    // 1: start with base polygon
    resultPolys->append(currentBasePolygonPoints());

    // 2: apply each enabled and valid filter ...
    //    ... each step may in general result in cutting the input polygons into one or more smaller polygons
    foreach (Filter *filter, filters_) {
        if ((!filter->enabledCheckBox_->isChecked()) || (!filter->isValid()))
            continue;
        PointVectors inPolys(resultPolys);
        resultPolys = PointVectors(new QVector<PointVector>());
        for (int i = 0; i < inPolys->size(); ++i) {
            if (inPolys->at(i)) {
                PointVectors outPolys = filter->apply(inPolys->at(i));
                for (int j = 0; j < outPolys->size(); ++j)
                    resultPolys->append(outPolys->at(j));
            }
        }

        if (resultPolys->isEmpty())
            break; // everything's been filtered away!
    }

    return resultPolys;
}

float ControlPanel::ballSizeFrac()
{
    return bsSlider_ ? (float(bsSlider_->value() - bsSlider_->minimum()) / (bsSlider_->maximum() - bsSlider_->minimum())) : 0.0;
}

Filter *ControlPanel::currentFilter() const
{
    foreach (Filter *filter, filters_)
        if (filter->currCheckBox_->isChecked())
            return filter;
    Q_ASSERT(false);
    return 0;
}

void ControlPanel::close()
{
    setVisible(false);
}

void ControlPanel::updateGLWidget()
{
    MainWindow::instance().glWidget()->updateGL();
}

void ControlPanel::basePolygonTypeChanged()
{
    customBasePolygonEditableOnSphereCheckBox_->setVisible(currentBasePolygonType() == BasePolygon::Custom);
    updateGLWidget();
}
