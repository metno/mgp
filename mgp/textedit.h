#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include <QBitArray>
#include <QPair>
#include <QHash>

class QMouseEvent;

class TextEdit : public QTextEdit
{
Q_OBJECT
public:
    TextEdit(QWidget *parent = 0);
    void resetHighlighting();
    void addMatchedRange(const QPair<int, int> &);
    void addIncompleteRange(const QPair<int, int> &, const QString &);
    void showHighlighting();

private:
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    QBitArray matched_;
    QBitArray incomplete_;
    QHash<int, QString> reason_;
};

#endif // TEXTEDIT_H
