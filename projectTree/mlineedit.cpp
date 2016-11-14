#include "mlineedit.h"
#include <QKeyEvent>
#include <QDebug>

mLineEdit::mLineEdit(QWidget *parent):QLineEdit(parent)
{
    setGeometry(QRect(QPoint(5,450), QSize(143,25)));
}

void mLineEdit::keyPressEvent(QKeyEvent *event)
{
    QLineEdit::keyPressEvent(event);
    return;
}
