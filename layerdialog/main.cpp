#include <QtGui>
#include "empty.xpm"
#include "visible.xpm"
#include "unsavedchanges.xpm"
#include "addempty.xpm"
#include "mergevisible.xpm"
#include "showall.xpm"
#include "hideall.xpm"
#include "duplicate.xpm"
#include "remove.xpm"
#include "moveup.xpm"
#include "movedown.xpm"
#include "edit.xpm"

class CheckableLabel : public QLabel
{
    Q_OBJECT
public:
    CheckableLabel(bool, const QPixmap &, const QString &, const QString &, bool = true);
    void setChecked(bool);
    bool isChecked() { return checked_; }
private:
    bool checked_;
    QPixmap pixmap_;
    QString checkedToolTip_;
    QString uncheckedToolTip_;
    bool clickable_;
    void mousePressEvent(QMouseEvent *);
signals:
    void mouseClicked(QMouseEvent *);
};

CheckableLabel::CheckableLabel(bool checked, const QPixmap &pixmap, const QString &checkedToolTip, const QString &uncheckedToolTip, bool clickable)
    : checked_(checked)
    , pixmap_(pixmap)
    , checkedToolTip_(checkedToolTip)
    , uncheckedToolTip_(uncheckedToolTip)
    , clickable_(clickable)
{
    setMargin(0);
    setChecked(checked_);
}

void CheckableLabel::setChecked(bool enabled)
{
    checked_ = enabled;
    if (checked_) {
        setPixmap(pixmap_);
        setToolTip(checkedToolTip_);
    } else {
        setPixmap(empty_xpm);
        setToolTip(uncheckedToolTip_);
    }
}

void CheckableLabel::mousePressEvent(QMouseEvent *event)
{
    if (clickable_ && (event->button() & Qt::LeftButton))
        setChecked(!checked_);
    emit mouseClicked(event);
}

class NameLabel : public QLabel
{
    Q_OBJECT
public:
    NameLabel(const QString &);
private:
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
signals:
    void mouseClicked(QMouseEvent *);
    void mouseDoubleClicked(QMouseEvent *);
};

NameLabel::NameLabel(const QString &name)
    : QLabel(name)
{
    setMargin(0);
}

void NameLabel::mousePressEvent(QMouseEvent *event)
{
    emit mouseClicked(event);
}

void NameLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit mouseDoubleClicked(event);
}

class Layer : public QWidget
{
    Q_OBJECT
public:
    Layer(const QString &, QWidget * = 0);
    void setName(const QString &);
    QString name() const { return nameLabel_->text(); }
    void setLayerVisible(bool); // Note that setVisible() and isVisible() are already used in QWidget!
    bool isLayerVisible() const { return visibleLabel_->isChecked(); }
    void setUnsavedChanges(bool); // Note that setVisible() and isVisible() are already used in QWidget!
    bool hasUnsavedChanges() const { return unsavedChangesLabel_->isChecked(); }
    void setSelected(bool);
    bool isSelected() const { return selected_; }
    void editName();
private:
    CheckableLabel *visibleLabel_;
    CheckableLabel *unsavedChangesLabel_;
    NameLabel *nameLabel_;
    bool selected_;
signals:
    void mouseClicked(QMouseEvent *);
    void mouseDoubleClicked(QMouseEvent *);
    void visibilityChanged();
};

Layer::Layer(const QString &name_, QWidget *parent)
    : QWidget(parent)
    , selected_(false)
{
    setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    visibleLabel_ = new CheckableLabel(
                true, visible_xpm, "the layer is visible\n(click to make it invisible)", "the layer is invisible\n(click to make it visible)");
    connect(visibleLabel_, SIGNAL(mouseClicked(QMouseEvent *)), SIGNAL(visibilityChanged()));
    mainLayout->addWidget(visibleLabel_);

    static int nn = 0;
    unsavedChangesLabel_ = new CheckableLabel(
                nn++ % 2, unsavedchanges_xpm, "the layer has unsaved changes\n(do ??? to save them)", "the layer does not have any unsaved changes", false);
    connect(unsavedChangesLabel_, SIGNAL(mouseClicked(QMouseEvent *)), SIGNAL(mouseClicked(QMouseEvent *)));
    mainLayout->addWidget(unsavedChangesLabel_);

    nameLabel_ = new NameLabel(name_);
    nameLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(nameLabel_, SIGNAL(mouseClicked(QMouseEvent *)), SIGNAL(mouseClicked(QMouseEvent *)));
    connect(nameLabel_, SIGNAL(mouseDoubleClicked(QMouseEvent *)), SIGNAL(mouseDoubleClicked(QMouseEvent *)));
    mainLayout->addWidget(nameLabel_);
}

void Layer::setName(const QString &name)
{
    nameLabel_->setText(name);
}

void Layer::setLayerVisible(bool visible)
{
    visibleLabel_->setChecked(visible);
}

void Layer::setUnsavedChanges(bool unsavedChanges)
{
    unsavedChangesLabel_->setChecked(unsavedChanges);
}

void Layer::setSelected(bool selected)
{
    selected_ = selected;
    const QString ssheet(selected_ ? "QLabel { background-color : #f27b4b; color : black; }" : "");
    visibleLabel_->setStyleSheet(ssheet);
    unsavedChangesLabel_->setStyleSheet(ssheet);
    nameLabel_->setStyleSheet(ssheet);
}

void Layer::editName()
{
    bool ok;
    const QString name = QInputDialog::getText(this, "Edit layer name", "Layer name:", QLineEdit::Normal, nameLabel_->text(), &ok);
    if (ok)
        setName(name);
}

class ScrollArea : public QScrollArea
{
public:
    ScrollArea(QWidget * = 0);
private:
    void keyPressEvent(QKeyEvent *);
};

ScrollArea::ScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
}

void ScrollArea::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}

class LayerDialog : public QDialog
{
    Q_OBJECT
public:
    LayerDialog(QWidget * = 0);
private:
    int nextLayerIndex_;
    QVBoxLayout *layersLayout_;
    ScrollArea *scrollArea_;
    QToolButton *addEmptyButton_;
    QToolButton *mergeVisibleButton_;
    QToolButton *showAllButton_;
    QToolButton *hideAllButton_;
    QToolButton *duplicateSelectedButton_;
    QToolButton *removeSelectedButton_;
    QToolButton *moveSelectedUpButton_;
    QToolButton *moveSelectedDownButton_;
    QToolButton *editSelectedButton_;
    QToolButton *createToolButton(const QIcon &, const QString &, const char *) const;
    void initialize(Layer *);
    void keyPressEvent(QKeyEvent *);
    int selectedIndex() const;
    Layer *selected();
    void selectIndex(int);
    void select(Layer *);
    Layer *atIndex(int);
    void ensureVisible(Layer *);
    void ensureSelectedVisible();
    void duplicate(Layer *);
    void remove(Layer *, bool = true);
    void remove(int);
    void moveUp(Layer *);
    void moveUp(int);
    void moveDown(Layer *);
    void moveDown(int);
    void setAllVisible(bool);
    QList<Layer *> visibleLayers();
private slots:
    void mouseClicked(QMouseEvent *);
    void mouseDoubleClicked(QMouseEvent *);
    void ensureSelectedVisibleTimeout();
    void addEmpty();
    void mergeVisible();
    void showAll();
    void hideAll();
    void duplicateSelected();
    void removeSelected();
    void moveSelectedUp();
    void moveSelectedDown();
    void editSelected();
    void updateButtons();
};

LayerDialog::LayerDialog(QWidget *parent)
    : QDialog(parent)
    , nextLayerIndex_(0)
{
    setWindowTitle("Layers");
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

//    QHBoxLayout *dummyTopLayout = new QHBoxLayout;
//    dummyTopLayout->addWidget(new QPushButton("dummy 1"));
//    dummyTopLayout->addWidget(new QPushButton("dummy 2"));
//    mainLayout->addLayout(dummyTopLayout);

    QWidget *layersWidget = new QWidget;
    layersWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    layersLayout_ = new QVBoxLayout(layersWidget);
    layersLayout_->setContentsMargins(0, 0, 0, 0);
    layersLayout_->setSpacing(0);
    layersLayout_->setMargin(0);
    for (int i = 0; i < 8; ++i) {
        Layer *layer = new Layer(QString("dummy name (%1)").arg(i));
        layersLayout_->addWidget(layer);
        initialize(layer);
    }

    scrollArea_ = new ScrollArea;
    scrollArea_->setWidget(layersWidget);
    scrollArea_->setWidgetResizable(true);

    mainLayout->addWidget(scrollArea_);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(addEmptyButton_ = createToolButton(QPixmap(addempty_xpm), "Add an empty layer", SLOT(addEmpty())));
    bottomLayout->addWidget(mergeVisibleButton_ = createToolButton(QPixmap(mergevisible_xpm), "Merge visible layers", SLOT(mergeVisible())));
    bottomLayout->addWidget(showAllButton_ = createToolButton(QPixmap(showall_xpm), "Show all layers", SLOT(showAll())));
    bottomLayout->addWidget(hideAllButton_ = createToolButton(QPixmap(hideall_xpm), "Hide all layers", SLOT(hideAll())));
    bottomLayout->addWidget(duplicateSelectedButton_ = createToolButton(QPixmap(duplicate_xpm), "Duplicate the selected layer", SLOT(duplicateSelected())));
    bottomLayout->addWidget(removeSelectedButton_ = createToolButton(QPixmap(remove_xpm), "Remove the selected layer", SLOT(removeSelected())));
    bottomLayout->addWidget(moveSelectedUpButton_ = createToolButton(QPixmap(moveup_xpm), "Move the selected layer up", SLOT(moveSelectedUp())));
    bottomLayout->addWidget(moveSelectedDownButton_ = createToolButton(QPixmap(movedown_xpm), "Move the selected layer down", SLOT(moveSelectedDown())));
    bottomLayout->addWidget(editSelectedButton_ = createToolButton(QPixmap(edit_xpm), "Edit the selected layer", SLOT(editSelected())));
    bottomLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
    mainLayout->addLayout(bottomLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(close()));
    mainLayout->addWidget(buttonBox);

    selectIndex(0);
}

QToolButton *LayerDialog::createToolButton(const QIcon &icon, const QString &toolTip, const char *method) const
{
    QToolButton *button = new QToolButton;
    button->setIcon(icon);
    button->setToolTip(toolTip);
    connect(button, SIGNAL(clicked()), this, method);
    return button;
}

void LayerDialog::initialize(Layer *layer)
{
    connect(layer, SIGNAL(mouseClicked(QMouseEvent *)), SLOT(mouseClicked(QMouseEvent *)));
    connect(layer, SIGNAL(mouseDoubleClicked(QMouseEvent *)), SLOT(mouseDoubleClicked(QMouseEvent *)));
    connect(layer, SIGNAL(visibilityChanged()), SLOT(updateButtons()));
}

void LayerDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Quit)) {
        qApp->quit();
    } else if (event->key() == Qt::Key_Up) {
        if (event->modifiers() & Qt::ControlModifier)
            moveSelectedUp();
        else
            selectIndex(selectedIndex() - 1);
    } else if (event->key() == Qt::Key_Down) {
        if (event->modifiers() & Qt::ControlModifier)
            moveSelectedDown();
        else
            selectIndex(selectedIndex() + 1);
    } else if ((event->key() == Qt::Key_Delete) || (event->key() == Qt::Key_Backspace)) {
        removeSelected();
    } else {
        QDialog::keyPressEvent(event);
    }
}

void LayerDialog::selectIndex(int index)
{
    select(atIndex(index));
}

void LayerDialog::select(Layer *layer)
{
    if (!layer)
        return;
    for (int i = 0; i < layersLayout_->count(); ++i) {
        Layer *layer_ = qobject_cast<Layer *>(layersLayout_->itemAt(i)->widget());
        layer_->setSelected(layer_ == layer);
    }
    ensureVisible(layer);
    updateButtons();
}

int LayerDialog::selectedIndex() const
{
    for (int i = 0; i < layersLayout_->count(); ++i)
        if (qobject_cast<Layer *>(layersLayout_->itemAt(i)->widget())->isSelected())
            return i;
    return -1;
}

Layer *LayerDialog::selected()
{
    return atIndex(selectedIndex());
}

Layer *LayerDialog::atIndex(int index)
{
    if (index >= 0 && index < layersLayout_->count())
        return qobject_cast<Layer *>(layersLayout_->itemAt(index)->widget());
    return 0;
}

void LayerDialog::duplicate(Layer *layer)
{
    const int index = layersLayout_->indexOf(layer);
    Layer *newLayer = new Layer(QString("%1 (duplicate) (%2)").arg(layer->name()).arg(nextLayerIndex_++));
    newLayer->setLayerVisible(layer->isLayerVisible());
    // copy contents of layer into newLayer ... 2 B DONE!
    layersLayout_->insertWidget(index + 1, newLayer);
    initialize(newLayer);
    select(newLayer);
    ensureSelectedVisible();
    updateButtons();
}

void LayerDialog::duplicateSelected()
{
    duplicate(selected());
}

void LayerDialog::remove(Layer *layer, bool confirm)
{
    if ((!layer) || (layersLayout_->count() == 0))
        return;

    if (confirm && (QMessageBox::warning(
                        this, "Remove layer", "Really remove layer?",
                        QMessageBox::Yes | QMessageBox::No) == QMessageBox::No))
        return;

    const int index = layersLayout_->indexOf(layer);
    layersLayout_->removeWidget(layer);
    delete layer;
    if (layersLayout_->count() > 0)
        selectIndex(qMin(index, layersLayout_->count() - 1));

    updateButtons();
}

void LayerDialog::remove(int index)
{
    remove(atIndex(index));
}

void LayerDialog::removeSelected()
{
    remove(selected());
}

void LayerDialog::moveUp(Layer *layer)
{
    const int index = layersLayout_->indexOf(layer);
    if (index <= 0)
        return;
    layersLayout_->removeWidget(layer);
    layersLayout_->insertWidget(index - 1, layer);
    if (layer == selected())
        ensureSelectedVisible();
    updateButtons();
}

void LayerDialog::moveUp(int index)
{
    moveUp(atIndex(index));
}

void LayerDialog::moveSelectedUp()
{
    moveUp(selected());
}

void LayerDialog::moveDown(Layer *layer)
{
    const int index = layersLayout_->indexOf(layer);
    if (index >= (layersLayout_->count() - 1))
        return;
    layersLayout_->removeWidget(layer);
    layersLayout_->insertWidget(index + 1, layer);
    if (layer == selected())
        ensureSelectedVisible();
    updateButtons();
}

void LayerDialog::moveDown(int index)
{
    moveDown(atIndex(index));
}

void LayerDialog::moveSelectedDown()
{
    moveDown(selected());
}

void LayerDialog::editSelected()
{
    selected()->editName(); // ### only the name for now
}

void LayerDialog::mouseClicked(QMouseEvent *event)
{
    Layer *layer = qobject_cast<Layer *>(sender());
    Q_ASSERT(layer);
    select(layer);
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
        //
        QAction editName_act(QPixmap(edit_xpm), tr("Edit Name"), 0);
        editName_act.setIconVisibleInMenu(true);

        // add actions
        contextMenu.addAction(&duplicate_act);
        contextMenu.addAction(&remove_act);
        contextMenu.addAction(&moveUp_act);
        contextMenu.addAction(&moveDown_act);
        contextMenu.addAction(&editName_act);
        QAction *action = contextMenu.exec(event->globalPos(), &duplicate_act);
        if (action == &duplicate_act) {
            duplicate(layer);
        } else if (action == &remove_act) {
            remove(layer);
        } else if (action == &moveUp_act) {
            moveUp(layer);
        } else if (action == &moveDown_act) {
            moveDown(layer);
        } else if (action == &editName_act) {
            layer->editName();
        }
    }
}

void LayerDialog::mouseDoubleClicked(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton)
        selected()->editName();
}

void LayerDialog::ensureVisible(Layer *layer)
{
    qApp->processEvents();
    scrollArea_->ensureWidgetVisible(layer);
}

void LayerDialog::ensureSelectedVisibleTimeout()
{
    ensureVisible(selected());
}

void LayerDialog::ensureSelectedVisible()
{
    QTimer::singleShot(0, this, SLOT(ensureSelectedVisibleTimeout()));
}

void LayerDialog::addEmpty()
{
    Layer *layer = new Layer(QString("empty layer (%1)").arg(nextLayerIndex_++));
    layersLayout_->addWidget(layer);
    initialize(layer);
    select(layer);
    ensureSelectedVisible();
    updateButtons();
}

void LayerDialog::mergeVisible()
{
    QList<Layer *> visLayers = visibleLayers();

    if (visLayers.size() > 1) {
        if (QMessageBox::warning(
                    this, "Merge visible layers", "Really merge visible layers?",
                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
            return;

        // mergeIntoFirst(visLayers); ... 2 B DONE!
        for (int i = 1; i < visLayers.size(); ++i)
            remove(visLayers.at(i), false);
        select(visLayers.first());
    }

    updateButtons();
}

void LayerDialog::setAllVisible(bool visible)
{
    for (int i = 0; i < layersLayout_->count(); ++i)
        qobject_cast<Layer *>(layersLayout_->itemAt(i)->widget())->setLayerVisible(visible);
    updateButtons();
}

void LayerDialog::showAll()
{
    setAllVisible(true);
}

void LayerDialog::hideAll()
{
    setAllVisible(false);
}

QList<Layer *> LayerDialog::visibleLayers()
{
    QList<Layer *> visLayers;
    for (int i = 0; i < layersLayout_->count(); ++i) {
        Layer *layer = qobject_cast<Layer *>(layersLayout_->itemAt(i)->widget());
        if (layer->isLayerVisible())
            visLayers.append(layer);
    }
    return visLayers;
}

void LayerDialog::updateButtons()
{
    mergeVisibleButton_->setEnabled(visibleLayers().size() > 1);
    showAllButton_->setEnabled(visibleLayers().size() < layersLayout_->count());
    hideAllButton_->setEnabled(visibleLayers().size() > 0);
    duplicateSelectedButton_->setEnabled(layersLayout_->count() > 0);
    removeSelectedButton_->setEnabled(layersLayout_->count() > 0);
    moveSelectedUpButton_->setEnabled(selectedIndex() > 0);
    moveSelectedDownButton_->setEnabled(selectedIndex() < (layersLayout_->count() - 1));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LayerDialog *dialog = new LayerDialog;
    dialog->resize(500, 400);
    dialog->show();

    return app.exec();
}

#include "main.moc"
