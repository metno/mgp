#include "controlpanel.h"
#include "common.h"
#include "mainwindow.h"
#include "glwidget.h"
#include "enor_fir.h"
#include "util3d.h"
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
#include <QLinkedList>
#include <QHash>
#include <algorithm>
#include <iostream> // ### for debugging

// Returns the SIGMET/AIRMET degrees form of a longitude value in radians ([-M_PI, M_PI]).
// Examples:
//        0 -> E000 (or E00000)
//    -M_PI -> W18000 (or W180)
// M_PI / 2 -> E09000 (or E090)
static QString xmetFormatLon(double lon_)
{
    double lon = RAD2DEG(fmod(lon_, 2 * M_PI));
    if (lon > 180)
        lon -= 360;
    lon = qMin(180.0, qMax(-180.0, lon));
    const int ipart = int(floor(fabs(lon)));
    const int fpart = int(floor(round(100 * (fabs(lon) - ipart))));
    return QString("%1%2%3").arg((lon < 0) ? "W" : "E").arg(ipart, 3, 10, QLatin1Char('0')).arg(fpart, 2, 10, QLatin1Char('0'));
}

// Returns the SIGMET/AIRMET degrees form of a latitude value in radians ([-M_PI / 2, M_PI / 2]).
// Examples:
//            0 -> N00 (or N0000)
//    -M_PI / 2 -> S9000 (or S90)
//     M_PI / 4 -> N4500 (or N45)
static QString xmetFormatLat(double lat_)
{
    double lat = RAD2DEG(fmod(lat_, M_PI));
    lat = qMin(90.0, qMax(-90.0, lat));
    const int ipart = int(floor(fabs(lat)));
    const int fpart = int(floor(round(100 * (fabs(lat) - ipart))));
    return QString("%1%2%3").arg(lat < 0 ? "S" : "N").arg(ipart, 2, 10, QLatin1Char('0')).arg(fpart, 2, 10, QLatin1Char('0'));
}

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
    case    WI: return  "WI";
    case  E_OF: return  "E OF";
    case  W_OF: return  "W OF";
    case  N_OF: return  "N OF";
    case  S_OF: return  "S OF";
    case NE_OF: return "NE OF LINE";
    case NW_OF: return "NW OF LINE";
    case SE_OF: return "SE OF LINE";
    case SW_OF: return "SW OF LINE";
    default: return "ERROR!";
    }
}

WithinFilter::WithinFilter(
        Type type, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox, const PointVector &defaultValue)
    : Filter(type, enabledCheckBox, currCheckBox)
    , points_(defaultValue)
{
}

Filter *WithinFilter::create(QGridLayout *layout, int row, Type type, const PointVector &defaultValue)
{
    WithinFilter *filter = new WithinFilter(type, new QCheckBox, new QCheckBox, defaultValue);

    QLabel *typeLabel = new QLabel(typeName(filter->type_));
    typeLabel->setStyleSheet("font-family:mono");
    layout->addWidget(typeLabel, row, 0, Qt::AlignRight);
    layout->addWidget(filter->enabledCheckBox_, row, 1, Qt::AlignHCenter);
    layout->addWidget(filter->currCheckBox_, row, 2, Qt::AlignHCenter);

    QFrame *valFrame = new QFrame;
    valFrame->setLayout(new QHBoxLayout);
    valFrame->layout()->addWidget(new QLabel("<coordinates accessible on sphere only>"));
    qobject_cast<QHBoxLayout *>(valFrame->layout())->addStretch(1);
    layout->addWidget(valFrame, row, 3);

    return filter;
}

QVariant WithinFilter::value() const
{
    return QVariant(); // for now
}

bool WithinFilter::startDragging(const QPair<double, double> &)
{
    return false;
}

void WithinFilter::updateDragging(const QPair<double, double> &)
{
}

bool WithinFilter::isValid() const
{
#if 0 // THE CODE BELOW CURRENTLY DOESN'T WORK, PROBABLY BECAUSE NEIGHBOUR EDGES ARE SOMETIMES CONSIDERED
      // AS INTERSECTING EVEN IF THEY ONLY TOUCH AT ENDPOINTS
    // check if polygon is self-intersecting by comparing each unique pair of edges
    for (int i = 0; i < (points_->size() - 1); ++i) {
        for (int j = i + 1; j < points_->size(); ++j) {
            if (Math::greatCircleArcsIntersect(
                        points_->at(i), points_->at(i + 1),
                        points_->at(j), points_->at((j + 1) % points_->size())))
                return false;
        }
    }
#endif

    return true;
}

bool WithinFilter::rejected(const QPair<double, double> &point) const
{
    return !Math::pointInPolygon(point, points_);
}

static PointVector reversed(const PointVector &points)
{
    PointVector copy(new QVector<QPair<double, double> >());
    for (int i = points->size() - 1; i >= 0; --i)
        copy->append(points->at(i));
    return copy;
}

struct IsctInfo {
    int isctId_; // non-negative intersection ID
    int c_; // intersection on line (c, (c + 1) % C.size()) in clip polygon C for 0 <= c < C.size()
    int s_; // intersection on line (s, (s + 1) % S.size()) in subject polygon S for 0 <= s < S.size()
    QPair<double, double> point_; // lon,lat radians of intersection point
    double cdist_; // distance between C->at(c) and point_
    double sdist_; // distance between S->at(s) and point_
    IsctInfo(int isctId, int c, int s, const QPair<double, double> &point, double cdist, double sdist)
        : isctId_(isctId)
        , c_(c)
        , s_(s)
        , point_(point)
        , cdist_(cdist)
        , sdist_(sdist)
    {}
};

struct Node {
    QPair<double, double> point_; // lon,lat radians of point represented by the node
    int isctId_; // non-negative intersection ID, or < 0 if the node does not represent an intersection
    QLinkedList<Node>::iterator neighbour_; // pointer to corresponding intersection node in other list
    bool entry_; // whether the intersection node represents an entry into (true) or exit from (false) the clipped polygon
    bool visited_; // whether the intersection node has already been processed in the generation of output polygons
    Node(const QPair<double, double> &point, int isctId = -1) : point_(point), isctId_(isctId), entry_(false), visited_(false) {}
};

static void printLists(const QString &tag, const QLinkedList<Node> &slist, const QLinkedList<Node> &clist)
{
    {
        std::cout << tag.toLatin1().data() << "; subj: ";
        QLinkedList<Node>::const_iterator it;
        for (it = slist.constBegin(); it != slist.constEnd(); ++it) {
            if (it->isctId_ < 0) {
                std::cout << "V  ";
            } else {
                std::cout << it->isctId_ << "<" << (it->entry_ ? "entry" : "exit") << ">  ";
                Q_ASSERT(it->isctId_ == it->neighbour_->isctId_);
            }
        }
        std::cout << std::endl;
    }

    {
        std::cout << tag.toLatin1().data() << "; clip: ";
        QLinkedList<Node>::const_iterator it;
        for (it = clist.constBegin(); it != clist.constEnd(); ++it) {
            if (it->isctId_ < 0) {
                std::cout << "V  ";
            } else {
                std::cout << it->isctId_ << "<" << (it->entry_ ? "entry" : "exit") << ">  ";
                Q_ASSERT(it->isctId_ == it->neighbour_->isctId_);
            }
        }
        std::cout << std::endl << std::endl;
    }
}

// This function implements the Greiner-Hormann clipping algorithm:
// - http://www.inf.usi.ch/hormann/papers/Greiner.1998.ECO.pdf
//
// See also:
// - https://en.wikipedia.org/wiki/Greiner%E2%80%93Hormann_clipping_algorithm
// - https://en.wikipedia.org/wiki/Weiler%E2%80%93Atherton_clipping_algorithm
// - https://www.jasondavies.com/maps/clip/
PointVectors WithinFilter::apply(const PointVector &inPoly) const
{
    // set up output polygons
    PointVectors outPolys = PointVectors(new QVector<PointVector>());

#if 0
    // set up input polygons and ensure they are oriented clockwise
    const PointVector C(Math::isClockwise(points_) ? points_ : reversed(points_)); // clip polygon
    Q_ASSERT(Math::isClockwise(C));
    const PointVector S(Math::isClockwise(inPoly) ? inPoly : reversed(inPoly)); // subject polygon
    Q_ASSERT(Math::isClockwise(S));
#else
    // set up input polygons regardless of orientation (seems to work fine)
    const PointVector C(points_); // clip polygon
    const PointVector S(inPoly); // subject polygon
#endif

    // find intersections
    int isctId = 0;
    QHash<int, QList<IsctInfo> > sIscts; // intersections for edges in S
    QHash<int, QList<IsctInfo> > cIscts; // intersections for edges in C
    for (int s = 0; s < S->size(); ++s) { // loop over vertices in S
        for (int c = 0; c < C->size(); ++c) { // loop over vertices in C
            QPair<double, double> isctPoint;
            if (Math::greatCircleArcsIntersect(S->at(s), S->at((s + 1) % S->size()), C->at(c), C->at((c + 1) % C->size()), &isctPoint)) {
                const IsctInfo isctInfo(
                            isctId++, c, s, isctPoint,
                            Math::distance(C->at(c), isctPoint),
                            Math::distance(S->at(s), isctPoint));

                // insert the intersection for the S edge in increasing distance from vertex s
                if (!sIscts.contains(s))
                    sIscts.insert(s, QList<IsctInfo>());
                QList<IsctInfo> &silist = sIscts[s];
                {
                    int i = 0;
                    for (; (i < silist.size()) && (silist.at(i).sdist_ < isctInfo.sdist_); ++i) ;
                    silist.insert(i, isctInfo);
                }

                // insert the intersection for the C edge in increasing distance from vertex c
                if (!cIscts.contains(c))
                    cIscts.insert(c, QList<IsctInfo>());
                QList<IsctInfo> &cilist = cIscts[c];
                {
                    int i = 0;
                    for (; (i < cilist.size()) && (cilist.at(i).cdist_ < isctInfo.cdist_); ++i) ;
                    cilist.insert(i, isctInfo);
                }
            }
        }
    }

    Q_ASSERT(!sIscts.isEmpty() || cIscts.isEmpty());
    Q_ASSERT(!cIscts.isEmpty() || sIscts.isEmpty());


    // ************************************************************************
    // * CASE 1: No intersections exist between clip and subject polygons     *
    // ************************************************************************
    if (sIscts.isEmpty()) {
        Q_ASSERT(cIscts.isEmpty());

        // compute number of subject points inside clip polygon
        int sPointsInC = 0;
        for (int i = 0; i < S->size(); ++i)
            if (Math::pointInPolygon(S->at(i), C))
                sPointsInC++;

        if (sPointsInC == S->size()) {
            // the subject polygon is completely enclosed within the clip polygon, so return a list with one item:
            // a deep copy (although implicitly shared for efficiency) of the subject polygon
            PointVector sCopy(new QVector<QPair<double, double> >(*S.data()));
            outPolys->append(sCopy);
            return outPolys;
        }

        // ### disable this test for now; the assertion could be caused by pointInPolygon() not being 100% robust
        //if (sPointsInC != 0)
        //    qDebug() << "WARNING: more than zero subject points inside clip polygon:" << sPointsInC;
        // Q_ASSERT(sPointsInC == 0); // otherwise there would be at least one intersection!

        // compute number of clip points inside subject polygon
        int cPointsInS = 0;
        for (int i = 0; i < C->size(); ++i)
            if (Math::pointInPolygon(C->at(i), S))
                cPointsInS++;

        if (cPointsInS == C->size()) {
            // the clip polygon is completely enclosed within the subject polygon, so return a list with one item:
            // a deep copy (although implicitly shared for efficiency) of the clip polygon
            PointVector cCopy(new QVector<QPair<double, double> >(*C.data()));
            outPolys->append(cCopy);
            return outPolys;
        }

        // ### disable this test for now; the assertion could be caused by pointInPolygon() not being 100% robust
        //if (cPointsInS != 0)
        //    qDebug() << "WARNING: more than zero clip points inside subject polygon:" << cPointsInS;
        // Q_ASSERT(cPointsInS == 0); // otherwise there would be at least one intersection!

        // at this point, the clip and subject polygons are completely disjoint, so return an empty list
        return outPolys;
    }


    // **********************************************************************
    // * CASE 2: Intersections exist between clip and subject polygons      *
    // **********************************************************************

    // *** PHASE 0: Create initial linked lists *********

    // create list with original vertices and intersections for subject polygon
    QLinkedList<Node> slist;
    for (int i = 0; i < S->size(); ++i) {
        // append node for point i
        slist.push_back(Node(S->at(i)));

        // append nodes for intersections on line (i, (i + 1) % S.size())
        if (sIscts.contains(i)) {
            const QList<IsctInfo> silist = sIscts.value(i);
            for (int j = 0; j < silist.size(); ++j) {
                Q_ASSERT((j == 0) || (silist.at(j - 1).sdist_ <= silist.at(j).sdist_));
                slist.push_back(Node(silist.at(j).point_, silist.at(j).isctId_));
            }
        }
    }

    // create list with original vertices and intersections for clip polygon
    QLinkedList<Node> clist;
    for (int i = 0; i < C->size(); ++i) {
        // append node for point i
        clist.push_back(Node(C->at(i)));

        // append nodes for intersections on line (i, (i + 1) % C.size())
        if (cIscts.contains(i)) {
            const QList<IsctInfo> cilist = cIscts.value(i);
            for (int j = 0; j < cilist.size(); ++j) {
                Q_ASSERT((j == 0) || (cilist.at(j - 1).cdist_ <= cilist.at(j).cdist_));
                clist.push_back(Node(cilist.at(j).point_, cilist.at(j).isctId_));
            }
        }
    }


    // *** PHASE 1: Connect corresponding intersection nodes *********

    {
        QLinkedList<Node>::iterator sit;
        for (sit = slist.begin(); sit != slist.end(); ++sit) {
            if (sit->isctId_ >= 0) {
                // find the corresponding intersection node in the clist and connect them together
                QLinkedList<Node>::iterator cit;
                for (cit = clist.begin(); cit != clist.end(); ++cit) {
                    if (cit->isctId_ == sit->isctId_) {
                        sit->neighbour_ = cit;
                        cit->neighbour_ = sit;
                    }
                }
            }
        }
    }


    // *** PHASE 2: Set entry/exit status for each intersection node *********

    {
        // whether the next intersection represents an entry into the clip polygon
        bool entry = !Math::pointInPolygon(slist.first().point_, C);

        QLinkedList<Node>::iterator it;
        for (it = slist.begin(); it != slist.end(); ++it) {
            if (it->isctId_ >= 0) {
                it->entry_ = entry;
                entry = !entry; // if this intersection was an entry, the next one must be an exit and vice versa
            }
        }
    }

    {
        // whether the next intersection represents an entry into the subject polygon
        bool entry = !Math::pointInPolygon(clist.first().point_, S);

        QLinkedList<Node>::iterator it;
        for (it = clist.begin(); it != clist.end(); ++it) {
            if (it->isctId_ >= 0) {
                it->entry_ = entry;
                entry = !entry; // if this intersection was an entry, the next one must be an exit and vice versa
            }
        }
    }

//    printLists(QString::number(nn), slist, clist);


    // *** PHASE 3: Generate clipped polygons *********
    {

        // loop over original vertices and intersections in subject polygon
        QLinkedList<Node>::iterator sit;
        for (sit = slist.begin(); sit != slist.end(); ++sit) {
            if ((sit->isctId_ >= 0) && (!sit->visited_)) {
                // this is an unvisited intersection, so start tracing a new polygon
                PointVector poly(new QVector<QPair<double, double> >());

                QLinkedList<Node>::iterator it(sit);
                bool forward = it->entry_;
                do {

                    // move one step along the current list
                    if (forward) {
                        it++;
                        if (it == slist.end())
                            it = slist.begin();
                        else if (it == clist.end())
                            it = clist.begin();
                    } else {
                        if (it == slist.begin())
                            it = slist.end();
                        else if (it == clist.begin())
                            it = clist.end();
                        it--;
                    }

                    if (it->isctId_ < 0) {
                        // original vertex
                        poly->append(it->point_); // append to new polygon
                    } else {
                        // intersection
                        poly->append(it->point_); // append to new polygon
                        it = it->neighbour_; // move to corresponding intersection in other list
                        it->visited_ = it->neighbour_->visited_ = true; // indicate that we're done with this intersection
                        forward = it->entry_; // update direction
                    }

                    // return an empty result if the algorithm has seemed to entered an infinite loop
                    // (this could for example happen when Math::greatCircleArcsIntersect() fails to find an intersection)
                    if (poly->size() > 2 * S->size() * C->size())
                        return PointVectors();

                } while (it->isctId_ != sit->isctId_); // as long as tracing has not got back to where it started

                if (poly->size() >= 3) // hm ... wouldn't this always be the case?
                    outPolys->append(poly);
            }
        }
    }

    return outPolys;
}

QVector<QPair<double, double> > WithinFilter::intersections(const QPair<double, double> &p1, const QPair<double, double> &p2) const
{
    QVector<QPair<double, double> > points;

    for (int i = 0; i < points_->size(); ++i) {
        QPair<double, double> isctPoint;
        if (Math::greatCircleArcsIntersect(points_->at(i), points_->at((i + 1) % points_->size()), p1, p2, &isctPoint))
            points.append(isctPoint);
    }

    return points; // FOR NOW
}

QString WithinFilter::xmetExpr() const
{
    QString s("WI");
    for (int i = 0; i < points_->size(); ++i)
        s += QString(" %1 %2").arg(xmetFormatLat(points_->at(i).second)).arg(xmetFormatLon(points_->at(i).first));
    return s;
}

LineFilter::LineFilter(Type type, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox)
    : Filter(type, enabledCheckBox, currCheckBox)
{
}

PointVectors LineFilter::apply(const PointVector &inPoly) const
{
    PointVectors outPolys = PointVectors(new QVector<PointVector>());

    // get rejection status for all points
    const int n = inPoly->size();
    QBitArray rej(n);
    for (int i = 0; i < n; ++i)
        rej[i] = rejected(inPoly->at(i));

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

QVector<QPair<double, double> > LineFilter::intersections(const QPair<double, double> &p1, const QPair<double, double> &p2) const
{
    QVector<QPair<double, double> > points;
    QPair<double, double> point;
    if ((rejected(p1) != rejected(p2)) && intersects(p1, p2, &point))
        points.append(point);
    return points;
}

LonOrLatFilter::LonOrLatFilter(
        Type type, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox, QDoubleSpinBox *valSpinBox, double defaultValue)
    : LineFilter(type, enabledCheckBox, currCheckBox)
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

bool LonOrLatFilter::startDragging(const QPair<double, double> &point)
{
    const double lon = RAD2DEG(point.first);
    const double lat = RAD2DEG(point.second);

    const double val = ((type_ == W_OF) || (type_ == E_OF)) ? lon : lat;
    valSpinBox_->setValue(val);

    dragged_ = true;
    return true;
}

void LonOrLatFilter::updateDragging(const QPair<double, double> &point)
{
    Q_ASSERT(dragged_);

    const double lon = RAD2DEG(point.first);
    const double lat = RAD2DEG(point.second);

    const double val = ((type_ == W_OF) || (type_ == E_OF)) ? lon : lat;
    valSpinBox_->setValue(val);
}

bool LonOrLatFilter::isValid() const
{
    return true; // ensured by the QSpinBox
}

bool LonOrLatFilter::rejected(const QPair<double, double> &point) const
{
    const double lon = RAD2DEG(point.first);
    const double lat = RAD2DEG(point.second);
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
    return Math::greatCircleArcsIntersect(p1, p2, qMakePair(lon, lat1), qMakePair(lon, lat2), isctPoint);
}

QString LonOrLatFilter::xmetExpr() const
{
    const double val = DEG2RAD(valSpinBox_->value());
    return QString("%1 %2").arg(typeName(type_)).arg(((type_ == N_OF) || (type_ == S_OF)) ? xmetFormatLat(val) : xmetFormatLon(val));
}

FreeLineFilter::FreeLineFilter(
        Type type, QCheckBox *enabledCheckBox, QCheckBox *currCheckBox,
        QDoubleSpinBox *lon1SpinBox, QDoubleSpinBox *lat1SpinBox, QDoubleSpinBox *lon2SpinBox, QDoubleSpinBox *lat2SpinBox,
        const QLineF &defaultValue)
    : LineFilter(type, enabledCheckBox, currCheckBox)
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

bool FreeLineFilter::startDragging(const QPair<double, double> &point)
{
    const double dist1 = Math::distance(point, qMakePair(DEG2RAD(lon1SpinBox_->value()), DEG2RAD(lat1SpinBox_->value())));
    const double dist2 = Math::distance(point, qMakePair(DEG2RAD(lon2SpinBox_->value()), DEG2RAD(lat2SpinBox_->value())));

    firstEndpointDragged_ = (dist1 < dist2);
    dragged_ = true;
    updateDragging(point);
    return true;
}

void FreeLineFilter::updateDragging(const QPair<double, double> &point)
{
    Q_ASSERT(dragged_);

    const double lon = RAD2DEG(point.first);
    const double lat = RAD2DEG(point.second);

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

bool FreeLineFilter::rejected(const QPair<double, double> &point) const
{
    const double lon0 = RAD2DEG(point.first);
    const double lat0 = RAD2DEG(point.second);
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

    const double crossDist = Math::crossTrackDistanceToGreatCircle(
                qMakePair(DEG2RAD(lon0), DEG2RAD(lat0)),
                qMakePair(DEG2RAD(lon1), DEG2RAD(lat1)),
                qMakePair(DEG2RAD(lon2), DEG2RAD(lat2)));

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
    return Math::greatCircleArcsIntersect(p1, p2, qMakePair(lon1, lat1), qMakePair(lon2, lat2), isctPoint);
}

QString FreeLineFilter::xmetExpr() const
{
    return QString("%1 %2 %3 - %4 %5").arg(typeName(type_))
            .arg(xmetFormatLat(DEG2RAD(lat1SpinBox_->value())))
            .arg(xmetFormatLon(DEG2RAD(lon1SpinBox_->value())))
            .arg(xmetFormatLat(DEG2RAD(lat2SpinBox_->value())))
            .arg(xmetFormatLon(DEG2RAD(lon2SpinBox_->value())));
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
    , basePolygonLinesVisibleCheckBox_(0)
    , basePolygonPointsVisibleCheckBox_(0)
    , basePolygonIntersectionsVisibleCheckBox_(0)
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

    QHBoxLayout *basePolygonLayout1 = new QHBoxLayout;
    basePolygonLayout->addLayout(basePolygonLayout1);

    basePolygonLinesVisibleCheckBox_ = new QCheckBox("Lines");
    basePolygonLinesVisibleCheckBox_->setChecked(true);
    connect(basePolygonLinesVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    basePolygonLayout1->addWidget(basePolygonLinesVisibleCheckBox_);

    basePolygonPointsVisibleCheckBox_ = new QCheckBox("Points");
    basePolygonPointsVisibleCheckBox_->setChecked(true);
    connect(basePolygonPointsVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    basePolygonLayout1->addWidget(basePolygonPointsVisibleCheckBox_);

    basePolygonIntersectionsVisibleCheckBox_ = new QCheckBox("Intersections");
    basePolygonIntersectionsVisibleCheckBox_->setChecked(true);
    connect(basePolygonIntersectionsVisibleCheckBox_, SIGNAL(stateChanged(int)), SLOT(updateGLWidget()));
    basePolygonLayout1->addWidget(basePolygonIntersectionsVisibleCheckBox_);

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
    filterLayout->addWidget(new QLabel("Value"), 2, 3, 1, 2, Qt::AlignLeft);
    for (int i = 0; i < 4; ++i)
        filterLayout->itemAtPosition(2, i)->widget()->setStyleSheet("font-weight:bold");

    // within filter (default value arbitrarily chosen for now)
    PointVector wiPoints (new QVector<QPair<double, double> >);
    wiPoints->append(qMakePair(0.17, 0.95));
    wiPoints->append(qMakePair(0.3, 1.02));
    wiPoints->append(qMakePair(0.25, 1.08));
    wiPoints->append(qMakePair(0.18, 1.08));
    filters_.insert(Filter::WI, WithinFilter::create(filterLayout, 3, Filter::WI, wiPoints));

    // lon|lat filters (default values arbitrarily chosen for now)
    filters_.insert(Filter::E_OF, LonOrLatFilter::create(filterLayout, 4, Filter::E_OF, 4));
    filters_.insert(Filter::W_OF, LonOrLatFilter::create(filterLayout, 5, Filter::W_OF, 12));
    filters_.insert(Filter::N_OF, LonOrLatFilter::create(filterLayout, 6, Filter::N_OF, 58));
    filters_.insert(Filter::S_OF, LonOrLatFilter::create(filterLayout, 7, Filter::S_OF, 66));

    // free line filters (default values arbitrarily chosen for now)
    filters_.insert(Filter::NE_OF, FreeLineFilter::create(filterLayout, 8, Filter::NE_OF, QLineF(QPointF(-4, 65), QPointF(14, 55))));
    filters_.insert(Filter::NW_OF, FreeLineFilter::create(filterLayout, 9, Filter::NW_OF, QLineF(QPointF(5, 53), QPointF(25, 64))));
    filters_.insert(Filter::SE_OF, FreeLineFilter::create(filterLayout, 10, Filter::SE_OF, QLineF(QPointF(-1, 54), QPointF(17, 67))));
    filters_.insert(Filter::SW_OF, FreeLineFilter::create(filterLayout, 11, Filter::SW_OF, QLineF(QPointF(-1, 67), QPointF(19, 57))));

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

    // --- END result polygons section -------------------------------------------


    // --- BEGIN SIGMET/AIRMET expression section -------------------------------------------
    QGroupBox *xmetExprGroupBox = new QGroupBox("SIGMET/AIRMET Expression");
    QVBoxLayout *xmetExprLayout = new QVBoxLayout;
    xmetExprGroupBox->setLayout(xmetExprLayout);
    mainLayout->addWidget(xmetExprGroupBox);

    xmetExprEdit_ = new QTextEdit;
    xmetExprLayout->addWidget(xmetExprEdit_);

    QHBoxLayout *xmetExprLayout2 = new QHBoxLayout;
    xmetExprLayout->addLayout(xmetExprLayout2);

    QPushButton *xmetExprFromFiltersButton = new QPushButton("Set expression from filters");
    xmetExprFromFiltersButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    connect(xmetExprFromFiltersButton, SIGNAL(clicked()), SLOT(setXmetExprFromFilters()));
    xmetExprLayout2->addWidget(xmetExprFromFiltersButton);

    QPushButton *filtersFromXmetExprButton = new QPushButton("Set filters from expression");
    filtersFromXmetExprButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    connect(filtersFromXmetExprButton, SIGNAL(clicked()), SLOT(setFiltersFromXmetExpr()));
    xmetExprLayout2->addWidget(filtersFromXmetExprButton);

    xmetExprLayout2->addStretch(1);

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

PointVector ControlPanel::WIFilterPoints() const
{
    return qobject_cast<WithinFilter *>(filters_.value(Filter::WI))->points_;
}

QVariant ControlPanel::value(Filter::Type type) const
{
    return filters_.value(type)->value();
}

bool ControlPanel::rejectedByAnyFilter(const QPair<double, double> &point) const
{
    foreach (Filter *filter, filters_) {
        if (filter->enabledCheckBox_->isChecked() && filter->rejected(point))
            return true;
    }
    return false;
}

// Returns all intersection points between enabled filters and the great circle segment between p1 and p2.
QVector<QPair<double, double> > ControlPanel::filterIntersections(const QPair<double, double> &p1, const QPair<double, double> &p2) const
{
    QVector<QPair<double, double> > points;
    foreach (Filter *filter, filters_) {
        if (filter->enabledCheckBox_->isChecked())
            points += filter->intersections(p1, p2);
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
bool ControlPanel::startFilterDragging(const QPair<double, double> &point) const
{
    if (!filtersEditableOnSphereCheckBox_->isChecked())
        return false; // wrong mode (hm ... should this be a Q_ASSERT() instead?)

    // ensure no filter is currently considered as being dragged
    foreach (Filter *filter, filters_)
        filter->dragged_ = false;

    // apply the operation to the current filter if it is enabled
    foreach (Filter *filter, filters_) {
        if (filter->currCheckBox_->isChecked())
            return (filter->enabledCheckBox_->isChecked() && filter->startDragging(point));
    }

    return false; // no match
}

// If there is a draggable filter, tell this filter to update its draggable control point with this pos, and update the GLWidget.
void ControlPanel::updateFilterDragging(const QPair<double, double> &point)
{
    foreach (Filter *filter, filters_) {
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

PointVector ControlPanel::currentBasePolygonPoints() const
{
    if (!basePolygonComboBox_)
        return PointVector();

    const BasePolygon::Type currType = static_cast<BasePolygon::Type>(basePolygonComboBox_->itemData(basePolygonComboBox_->currentIndex()).toInt());
    Q_ASSERT(basePolygons_.contains(currType));
    return basePolygons_.value(currType)->points_;
}

static int currentPolygonPoint(const PointVector &points, const QPair<double, double> &point, double tolerance)
{
    for (int i = 0; i < points->size(); ++i) {
        const double dist = Math::distance(point, points->at(i));
        if (dist < tolerance)
            return i;
    }
    return -1;
}

int ControlPanel::currentWIFilterPoint(const QPair<double, double> &point, double tolerance)
{
    return currentPolygonPoint(qobject_cast<WithinFilter *>(filters_.value(Filter::WI))->points_, point, tolerance);
}

int ControlPanel::currentCustomBasePolygonPoint(const QPair<double, double> &point, double tolerance)
{
    if (currentBasePolygonType() != BasePolygon::Custom)
        return -1;    
    return currentPolygonPoint(basePolygons_.value(BasePolygon::Custom)->points_, point, tolerance);
}

bool ControlPanel::customBasePolygonEditableOnSphere() const
{
    return customBasePolygonEditableOnSphereCheckBox_->isChecked();
}

void ControlPanel::updatePolygonPointDragging(PointVector &points, int index, const QPair<double, double> &point)
{
    Q_ASSERT((index >= 0) && (index < points->size()));
    (*points)[index] = point;
    updateGLWidget();
}

void ControlPanel::updateWIFilterPointDragging(int index, const QPair<double, double> &point)
{
    updatePolygonPointDragging(qobject_cast<WithinFilter *>(filters_.value(Filter::WI))->points_, index, point);
}

void ControlPanel::updateCustomBasePolygonPointDragging(int index, const QPair<double, double> &point)
{
    updatePolygonPointDragging(basePolygons_.value(BasePolygon::Custom)->points_, index, point);
}

void ControlPanel::addPointToPolygon(PointVector &points, int index)
{
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

void ControlPanel::addPointToWIFilter(int index)
{
    addPointToPolygon(qobject_cast<WithinFilter *>(filters_.value(Filter::WI))->points_, index);
    MainWindow::instance().glWidget()->updateWIFilterPoint();
    updateGLWidget();
}

void ControlPanel::addPointToCustomBasePolygon(int index)
{
    addPointToPolygon(basePolygons_.value(BasePolygon::Custom)->points_, index);
    MainWindow::instance().glWidget()->updateCurrCustomBasePolygonPoint();
    updateGLWidget();
}

void ControlPanel::removePointFromPolygon(PointVector &points, int index)
{
    Q_ASSERT((index >= 0) && (index < points->size()));

    if (points->size() <= 3)
        return; // we need to have at least a triangle!

    points->remove(index);
    MainWindow::instance().glWidget()->updateCurrCustomBasePolygonPoint();
    updateGLWidget();
}

void ControlPanel::removePointFromWIFilter(int index)
{
    removePointFromPolygon(qobject_cast<WithinFilter *>(filters_.value(Filter::WI))->points_, index);
}

void ControlPanel::removePointFromCustomBasePolygon(int index)
{
    removePointFromPolygon(basePolygons_.value(BasePolygon::Custom)->points_, index);
}

// Returns true iff the current base polygon is oriented clockwise.
bool ControlPanel::currentBasePolygonIsClockwise() const
{
    return Math::isClockwise(currentBasePolygonPoints());
}

// Returns true iff the given point is considered inside the current base polygon.
bool ControlPanel::withinCurrentBasePolygon(const QPair<double, double> &point) const
{
    return Math::pointInPolygon(point, currentBasePolygonPoints());
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

        // set the input polygons for this filter to be the output polygons from the previous filter
        PointVectors inPolys(resultPolys);

        // create an empty list of output polygons to be filled in by this filter
        resultPolys = PointVectors(new QVector<PointVector>());

        // loop over input polygons
        for (int i = 0; i < inPolys->size(); ++i) {
            if (inPolys->at(i)) {
                // apply the filter to convert this input polygon to zero or more output polygons and add the to the output list
                PointVectors outPolys = filter->apply(inPolys->at(i));
                if (outPolys) {
                    for (int j = 0; j < outPolys->size(); ++j)
                        resultPolys->append(outPolys->at(j));
                }
            }
        }

        if (resultPolys->isEmpty())
            break; // everything's been filtered away!
    }

    return resultPolys;
}

void ControlPanel::updateResultPolygonsGroupBoxTitle(int n)
{
    resultPolygonsGroupBox_->setTitle(QString("Result Polygons%1").arg(n < 0 ? QString() : QString(" (%1)").arg(n)));
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

// Sets a canonical SIGMET/AIRMET expression from the current filters.
void ControlPanel::setXmetExprFromFilters()
{
    QString s;
    bool addSpace = false;
    foreach (Filter *filter, filters_) {
        if (filter->enabledCheckBox_->isChecked() && filter->isValid()) {
            if (addSpace)
                s += " ";
            addSpace = true; // add a space from now on
            s += filter->xmetExpr();
        }
    }
    xmetExprEdit_->setPlainText(s);
}

// Sets the filters from the SIGMET/AIRMET expression if possible.
void ControlPanel::setFiltersFromXmetExpr()
{
    qDebug() << "setting filters from expression:" << xmetExprEdit_->toPlainText().trimmed() << " (not implemented)";
}
