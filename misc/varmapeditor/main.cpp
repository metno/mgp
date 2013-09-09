#include <QtGui>
#include <limits>

static QWidget * createEditor(const QVariant &val)
{
  QWidget *editor = 0;
  if ((val.type() == QVariant::Double) || (val.type() == QVariant::Int) || (val.type() == QVariant::String)) {
    editor = new QLineEdit;
    qobject_cast<QLineEdit *>(editor)->setText(val.toString());
  } else {
    qDebug() << "WARNING: unsupported type:" << val.typeName();
    editor = new QLineEdit;
    qobject_cast<QLineEdit *>(editor)->setText(QString("UNSUPPORTED TYPE:").arg(val.typeName()));
  }
  return editor;
}

class VarMapEditor : public QDialog
{
public:
  static VarMapEditor *instance()
  {
    if (!instance_)
      instance_ = new VarMapEditor;
    return instance_;
  }

  // Opens the editor panel with initial values.
  // Returns the edited values (always unchanged when the dialog is cancelled).
  QVariantMap edit(const QVariantMap &values)
  {
    // clear old content
    if (glayout_->columnCount() == 2) {
      Q_ASSERT((glayout_->rowCount() > 0));
      for (int i = 0; i < glayout_->rowCount(); ++i) {
        QLayoutItem *keyItem = glayout_->itemAtPosition(i, 0);
        Q_ASSERT(keyItem);
        Q_ASSERT(keyItem->widget());
        Q_ASSERT(qobject_cast<QLabel *>(keyItem->widget()));
        delete keyItem->widget();
        // delete keyItem; ???

        QLayoutItem *valItem = glayout_->itemAtPosition(i, 1);
        Q_ASSERT(valItem);
        Q_ASSERT(valItem->widget());
        delete valItem->widget();
        // delete valItem; ???
      }
    }

    // set new content and initial values
    int row = 0;
    foreach (const QString key, values.keys()) {
      QLabel *label = new QLabel(key);
      //label->setStyleSheet("QLabel { background-color:#ff0 }");
      label->setAlignment(Qt::AlignRight);
      glayout_->addWidget(label, row, 0);
      QWidget *editor = createEditor(values.value(key));
      glayout_->addWidget(editor, row, 1);
      row++;
    }

    // open dialog
    if (exec()== QDialog::Accepted) {
      // return edited values
      QVariantMap newValues;
      for (int i = 0; i < glayout_->rowCount(); ++i) {
        const QString key = qobject_cast<const QLabel *>(glayout_->itemAtPosition(i, 0)->widget())->text();
        QWidget *editor = glayout_->itemAtPosition(i, 1)->widget();
        if (qobject_cast<QLineEdit *>(editor)) {
          newValues.insert(key, qobject_cast<const QLineEdit *>(editor)->text());
        } else {
          // add more editor types ... 2 B DONE
        }
      }
      return newValues;
    }

    return values; // return original values
  }

private:
  VarMapEditor()
  {
    QVBoxLayout *layout = new QVBoxLayout;
    glayout_ = new QGridLayout;
    layout->addLayout(glayout_);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    layout->addWidget(buttonBox);

    setLayout(layout);
  }

  static VarMapEditor *instance_;
  QGridLayout *glayout_;
};

VarMapEditor *VarMapEditor::instance_ = 0;


class Window : public QWidget
{
  Q_OBJECT
public:
  Window()
  {
    setLayout(new QVBoxLayout);
    QPushButton *b = new QPushButton("open editor");
    connect(b, SIGNAL(clicked()), this, SLOT(openVarMapEditor()));
    layout()->addWidget(b);
  }

public slots:
  void openVarMapEditor()
  {
    QVariantMap values;
    values.insert("int1", 4711);
    values.insert("string1", "bla bla");
    values.insert("real1", 3.14);

    const QVariantMap newValues = VarMapEditor::instance()->edit(values);
    if (newValues != values)
      qDebug() << "changed:" << newValues;
    else
      qDebug() << "unchanged" << newValues;
  }
};


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  Window window;
  window.show();

  window.openVarMapEditor();

  return app.exec();
}

#include "main.moc"
