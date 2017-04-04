#include "xmetareaeditdialog.h"
#include "pixmaps/addnew.xpm"
#include "pixmaps/duplicate.xpm"
#include "pixmaps/remove.xpm"
#include "pixmaps/moveup.xpm"
#include "pixmaps/movedown.xpm"
#include "xmetareaedit.h"
#include "mgp.h"
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QSpinBox>
#include <QToolButton>
#include <QKeyEvent>
#include <QPushButton>
#include <QCoreApplication>
#include <QTimer>
#include <QMenu>
#include <QVariantList>

#include <QDebug>

SelLabel::SelLabel()
    : QLabel(0)
{
    setMargin(0);
}

void SelLabel::mousePressEvent(QMouseEvent *event)
{
    emit mouseClicked(event);
}

PointEdit::PointEdit(LonDir lonDir_, int lonDeg_, int lonSec_, LatDir latDir_, int latDeg_, int latSec_, QWidget *parent)
    : QWidget(parent)
    , selected_(false)
{
    init(lonDir_, lonDeg_, lonSec_, latDir_, latDeg_, latSec_);
}

PointEdit::PointEdit(const PointEdit &other)
    : QWidget(other.parentWidget())
{
    init(other.lonDirEdit_->itemData(other.lonDirEdit_->currentIndex()).value<LonDir>(),
         other.lonDegEdit_->value(),
         other.lonSecEdit_->value(),
         other.latDirEdit_->itemData(other.latDirEdit_->currentIndex()).value<LatDir>(),
         other.latDegEdit_->value(),
         other.latSecEdit_->value());
}

void PointEdit::init(LonDir lonDir_, int lonDeg_, int lonSec_, LatDir latDir_, int latDeg_, int latSec_)
{
    setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    selLabel_ = new SelLabel;
    selLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    selLabel_->setMinimumWidth(30);
    selLabel_->setAlignment(Qt::AlignRight);
    connect(selLabel_, SIGNAL(mouseClicked(QMouseEvent *)), SIGNAL(mouseClicked(QMouseEvent *)));
    mainLayout->addWidget(selLabel_);

    spacingLabel1_ = new SelLabel;
    spacingLabel1_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacingLabel1_->setMinimumWidth(10);
    connect(spacingLabel1_, SIGNAL(mouseClicked(QMouseEvent *)), SIGNAL(mouseClicked(QMouseEvent *)));
    mainLayout->addWidget(spacingLabel1_);

    latDirEdit_ = new QComboBox;
    latDirEdit_->addItem("N", N);
    latDirEdit_->addItem("S", S);
    latDirEdit_->setCurrentIndex(latDirEdit_->findData(latDir_));
    mainLayout->addWidget(latDirEdit_);

    latSecEdit_ = new QSpinBox;
    latSecEdit_->setAlignment(Qt::AlignRight);
    latSecEdit_->setRange(0, 59);
    latSecEdit_->setValue(latSec_);
    connect(latSecEdit_, SIGNAL(valueChanged(int)), SLOT(handleLatSecValueChanged(int)));

    latDegEdit_ = new QSpinBox;
    latDegEdit_->setAlignment(Qt::AlignRight);
    latDegEdit_->setRange(0, 90);
    latDegEdit_->setValue(latDeg_);
    handleLatSecValueChanged(latSecEdit_->value());

    mainLayout->addWidget(latDegEdit_);
    mainLayout->addWidget(latSecEdit_);

    spacingLabel2_ = new QLabel;
    spacingLabel2_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacingLabel2_->setMinimumWidth(10);
    mainLayout->addWidget(spacingLabel2_);

    lonDirEdit_ = new QComboBox;
    lonDirEdit_->addItem("E", E);
    lonDirEdit_->addItem("W", W);
    lonDirEdit_->setCurrentIndex(lonDirEdit_->findData(lonDir_));
    mainLayout->addWidget(lonDirEdit_);

    lonSecEdit_ = new QSpinBox;
    lonSecEdit_->setAlignment(Qt::AlignRight);
    lonSecEdit_->setRange(0, 59);
    lonSecEdit_->setValue(lonSec_);
    connect(lonSecEdit_, SIGNAL(valueChanged(int)), SLOT(handleLonSecValueChanged(int)));

    lonDegEdit_ = new QSpinBox;
    lonDegEdit_->setAlignment(Qt::AlignRight);
    lonDegEdit_->setRange(0, 180);
    lonDegEdit_->setValue(lonDeg_);
    handleLonSecValueChanged(lonSecEdit_->value());

    mainLayout->addWidget(lonDegEdit_);
    mainLayout->addWidget(lonSecEdit_);
}

int PointEdit::lonDir() const
{
    return lonDirEdit_->itemData(lonDirEdit_->currentIndex()).toInt();
}

int PointEdit::lonDeg() const
{
    return lonDegEdit_->value();
}

int PointEdit::lonSec() const
{
    return lonSecEdit_->value();
}

int PointEdit::latDir() const
{
    return latDirEdit_->itemData(latDirEdit_->currentIndex()).toInt();
}

int PointEdit::latDeg() const
{
    return latDegEdit_->value();
}

int PointEdit::latSec() const
{
    return latSecEdit_->value();
}

void PointEdit::setSelected(bool selected)
{
    selected_ = selected;
    const QString ssheet(selected_ ? "QLabel { background-color : #f27b4b; color : black; }" : "");
    selLabel_->setStyleSheet(ssheet);
    spacingLabel1_->setStyleSheet(ssheet);
    //spacingLabel2_->setStyleSheet(ssheet);
}

void PointEdit::setLabelText(const QString &text)
{
    selLabel_->setText(text);
}

void PointEdit::handleLonSecValueChanged(int val)
{
    lonDegEdit_->setMaximum((val == 0) ? 180 : 179);
}

void PointEdit::handleLatSecValueChanged(int val)
{
    latDegEdit_->setMaximum((val == 0) ? 90 : 89);
}

ScrollArea::ScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
}

void ScrollArea::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}

XMETAreaEditDialog::XMETAreaEditDialog(XMETAreaEdit *xmetAreaEdit, QWidget *parent)
    : QDialog(parent)
    , xmetAreaEdit_(xmetAreaEdit)
{
    setWindowTitle("Points");
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(400, 300);

    //QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;

    QWidget *pointsWidget = new QWidget;
    pointsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    pointsLayout_ = new QVBoxLayout(pointsWidget);
    pointsLayout_->setContentsMargins(0, 0, 0, 0);
    pointsLayout_->setSpacing(0);
    pointsLayout_->setMargin(0);

    scrollArea_ = new ScrollArea;
    scrollArea_->setWidget(pointsWidget);
    scrollArea_->setWidgetResizable(true);

    mainLayout->addWidget(scrollArea_);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(addNewButton_ = createToolButton(QPixmap(addnew_xpm), "Add a new point", SLOT(addNew())));
    bottomLayout->addWidget(duplicateSelectedButton_ = createToolButton(QPixmap(duplicate_xpm), "Duplicate the selected point", SLOT(duplicateSelected())));
    bottomLayout->addWidget(removeSelectedButton_ = createToolButton(QPixmap(remove_xpm), "Remove the selected point", SLOT(removeSelected())));
    bottomLayout->addWidget(moveSelectedUpButton_ = createToolButton(QPixmap(moveup_xpm), "Move the selected point up", SLOT(moveSelectedUp())));
    bottomLayout->addWidget(moveSelectedDownButton_ = createToolButton(QPixmap(movedown_xpm), "Move the selected point down", SLOT(moveSelectedDown())));
    bottomLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
    mainLayout->addLayout(bottomLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);    
}

QToolButton *XMETAreaEditDialog::createToolButton(const QIcon &icon, const QString &toolTip, const char *method) const
{
    QToolButton *button = new QToolButton;
    button->setIcon(icon);
    button->setToolTip(toolTip);
    connect(button, SIGNAL(clicked()), this, method);
    return button;
}

void XMETAreaEditDialog::initialize(PointEdit *pointEdit)
{
    connect(pointEdit, SIGNAL(mouseClicked(QMouseEvent *)), SLOT(mouseClicked(QMouseEvent *)));
}

// Opens modal dialog based on current contents of xmetAreaEdit_.
// Upon clicking Cancel, dialog closes and contents of xmetAreaExit_ is unmodified.
// Upon clicking Ok, dialog closes and contents of xmetAreaExit_ is modified.
void XMETAreaEditDialog::edit(bool wiKeywordImplicit)
{
    // clear existing contents
    while (pointsLayout_->count())
        remove(0);

    // set new contents and initial values
    const mgp::Filters filters = xmetAreaEdit_->filters();
    foreach (mgp::Filter filter, *filters) {
        if (filter->type() == mgp::FilterBase::WI) {
            const QVariantList list = filter->toVariant().toList();
            Q_ASSERT(!(list.size() % 2));
            for (int i = 0; i < list.size(); i += 2) {
                bool ok = false;

                const double lon = list.at(i).toDouble(&ok);
                Q_ASSERT(ok);
                const QString lonExpr = mgp::xmetFormatLon(lon);
                const PointEdit::LonDir lonDir = (lonExpr.at(0).toUpper() == QLatin1Char('E')) ? PointEdit::E : PointEdit::W;
                const int lonDeg = lonExpr.mid(1, 3).toInt(&ok);
                Q_ASSERT(ok);
                const int lonSec = lonExpr.mid(4, 2).toInt(&ok);
                Q_ASSERT(ok);

                const double lat = list.at(i + 1).toDouble(&ok);
                Q_ASSERT(ok);
                const QString latExpr = mgp::xmetFormatLat(lat);
                const PointEdit::LatDir latDir = (latExpr.at(0).toUpper() == QLatin1Char('N')) ? PointEdit::N : PointEdit::S;
                const int latDeg = latExpr.mid(1, 2).toInt(&ok);
                Q_ASSERT(ok);
                const int latSec = latExpr.mid(3, 2).toInt(&ok);
                Q_ASSERT(ok);

                PointEdit *pointEdit = new PointEdit(lonDir, lonDeg, lonSec, latDir, latDeg, latSec);
                pointsLayout_->addWidget(pointEdit);
                initialize(pointEdit);
            }
            select(atIndex(0));
            break;
        }
    }

    updateButtonsAndLabels();

    // open dialog
    if (exec()== QDialog::Accepted) {
        QString s;
        for (int i = 0; i < pointsLayout_->count(); ++i) {
            PointEdit *pointEdit = qobject_cast<PointEdit *>(pointsLayout_->itemAt(i)->widget());

            if (i > 0) s += " - ";

            s += (pointEdit->latDir() == PointEdit::N) ? "N" : "S";
            s += QString("%1").arg(pointEdit->latDeg(), 2, 10, QLatin1Char('0'));
            s += QString("%1 ").arg(pointEdit->latSec(), 2, 10, QLatin1Char('0'));

            s += (pointEdit->lonDir() == PointEdit::E) ? "E" : "W";
            s += QString("%1").arg(pointEdit->lonDeg(), 3, 10, QLatin1Char('0'));
            s += QString("%1").arg(pointEdit->lonSec(), 2, 10, QLatin1Char('0'));
        }

        if (!wiKeywordImplicit)
            s.prepend("WI ");

        if (!s.isEmpty()) {
            xmetAreaEdit_->setPlainText(s);
        }
    }
}

void XMETAreaEditDialog::selectIndex(int index)
{
    select(atIndex(index));
}

void XMETAreaEditDialog::select(PointEdit *pointEdit)
{
    if (!pointEdit)
        return;
    for (int i = 0; i < pointsLayout_->count(); ++i) {
        PointEdit *pointEdit_ = qobject_cast<PointEdit *>(pointsLayout_->itemAt(i)->widget());
        pointEdit_->setSelected(pointEdit_ == pointEdit);
    }
    ensureVisible(pointEdit);
    updateButtonsAndLabels();
}

int XMETAreaEditDialog::selectedIndex() const
{
    for (int i = 0; i < pointsLayout_->count(); ++i)
        if (qobject_cast<PointEdit *>(pointsLayout_->itemAt(i)->widget())->isSelected())
            return i;
    return -1;
}

PointEdit *XMETAreaEditDialog::selected()
{
    return atIndex(selectedIndex());
}

PointEdit *XMETAreaEditDialog::atIndex(int index)
{
    if (index >= 0 && index < pointsLayout_->count())
        return qobject_cast<PointEdit *>(pointsLayout_->itemAt(index)->widget());
    return 0;
}

void XMETAreaEditDialog::addNew()
{
    PointEdit *pointEdit = new PointEdit;
    pointsLayout_->addWidget(pointEdit);
    initialize(pointEdit);
    select(pointEdit);
    ensureSelectedVisible();
    updateButtonsAndLabels();
}

void XMETAreaEditDialog::duplicate(PointEdit *pointEdit)
{
    const int index = pointsLayout_->indexOf(pointEdit);
    PointEdit *newPointEdit = new PointEdit(*pointEdit);
    pointsLayout_->insertWidget(index + 1, newPointEdit);
    initialize(newPointEdit);
    select(newPointEdit);
    ensureSelectedVisible();
    updateButtonsAndLabels();
}

void XMETAreaEditDialog::duplicateSelected()
{
    duplicate(selected());
}

void XMETAreaEditDialog::remove(PointEdit *pointEdit)
{
    if ((!pointEdit) || (pointsLayout_->count() == 0))
        return;

    const int index = pointsLayout_->indexOf(pointEdit);
    pointsLayout_->removeWidget(pointEdit);
    delete pointEdit;
    if (pointsLayout_->count() > 0)
        selectIndex(qMin(index, pointsLayout_->count() - 1));

    updateButtonsAndLabels();
}

void XMETAreaEditDialog::remove(int index)
{
    remove(atIndex(index));
}

void XMETAreaEditDialog::removeSelected()
{
    remove(selected());
}

void XMETAreaEditDialog::moveUp(PointEdit *pointEdit)
{
    const int index = pointsLayout_->indexOf(pointEdit);
    if (index <= 0)
        return;
    pointsLayout_->removeWidget(pointEdit);
    pointsLayout_->insertWidget(index - 1, pointEdit);
    if (pointEdit == selected())
        ensureSelectedVisible();
    updateButtonsAndLabels();
}

void XMETAreaEditDialog::moveUp(int index)
{
    moveUp(atIndex(index));
}

void XMETAreaEditDialog::moveSelectedUp()
{
    moveUp(selected());
}

void XMETAreaEditDialog::moveDown(PointEdit *pointEdit)
{
    const int index = pointsLayout_->indexOf(pointEdit);
    if (index >= (pointsLayout_->count() - 1))
        return;
    pointsLayout_->removeWidget(pointEdit);
    pointsLayout_->insertWidget(index + 1, pointEdit);
    if (pointEdit == selected())
        ensureSelectedVisible();
    updateButtonsAndLabels();
}

void XMETAreaEditDialog::moveDown(int index)
{
    moveDown(atIndex(index));
}

void XMETAreaEditDialog::moveSelectedDown()
{
    moveDown(selected());
}

void XMETAreaEditDialog::updateButtonsAndLabels()
{
    // update buttons
    duplicateSelectedButton_->setEnabled(pointsLayout_->count() > 0);
    removeSelectedButton_->setEnabled(pointsLayout_->count() > 0);
    moveSelectedUpButton_->setEnabled(selectedIndex() > 0);
    moveSelectedDownButton_->setEnabled(selectedIndex() < (pointsLayout_->count() - 1));

    // update labels
    for (int i = 0; i < pointsLayout_->count(); ++i) {
        PointEdit *pointEdit = qobject_cast<PointEdit *>(pointsLayout_->itemAt(i)->widget());
        pointEdit->setLabelText(QString("%1").arg(i + 1));
    }
}

void XMETAreaEditDialog::mouseClicked(QMouseEvent *event)
{
    PointEdit *pointEdit = qobject_cast<PointEdit *>(sender());
    Q_ASSERT(pointEdit);
    select(pointEdit);
    if (event->button() & Qt::RightButton) {
        QMenu contextMenu;
        QAction duplicate_act(QPixmap(duplicate_xpm), tr("Duplicate"), 0);
        duplicate_act.setIconVisibleInMenu(true);
        duplicate_act.setEnabled(duplicateSelectedButton_->isEnabled());
        //
        QAction remove_act(QPixmap(remove_xpm), tr("Remove"), 0);
        remove_act.setIconVisibleInMenu(true);
        remove_act.setEnabled(removeSelectedButton_->isEnabled());
        //
        QAction moveUp_act(QPixmap(moveup_xpm), tr("Move Up"), 0);
        moveUp_act.setIconVisibleInMenu(true);
        moveUp_act.setEnabled(moveSelectedUpButton_->isEnabled());
        //
        QAction moveDown_act(QPixmap(movedown_xpm), tr("Move Down"), 0);
        moveDown_act.setIconVisibleInMenu(true);
        moveDown_act.setEnabled(moveSelectedDownButton_->isEnabled());

        // add actions
        contextMenu.addAction(&duplicate_act);
        contextMenu.addAction(&remove_act);
        contextMenu.addAction(&moveUp_act);
        contextMenu.addAction(&moveDown_act);
        QAction *action = contextMenu.exec(event->globalPos(), &duplicate_act);
        if (action == &duplicate_act) {
            duplicate(pointEdit);
        } else if (action == &remove_act) {
            remove(pointEdit);
        } else if (action == &moveUp_act) {
            moveUp(pointEdit);
        } else if (action == &moveDown_act) {
            moveDown(pointEdit);
        }
    }
}

void XMETAreaEditDialog::ensureVisible(PointEdit *pointEdit)
{
    qApp->processEvents();
    scrollArea_->ensureWidgetVisible(pointEdit);
}

void XMETAreaEditDialog::ensureSelectedVisibleTimeout()
{
    ensureVisible(selected());
}

void XMETAreaEditDialog::ensureSelectedVisible()
{
    QTimer::singleShot(0, this, SLOT(ensureSelectedVisibleTimeout()));
}
