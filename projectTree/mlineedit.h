#ifndef MLINEEDIT_H
#define MLINEEDIT_H


#include <QLineEdit>


class mLineEdit : public QLineEdit
{
public:
    mLineEdit(QWidget *parent = 0);

public slots:
    void keyPressEvent(QKeyEvent *event);

signals:
    void searchSignal(QString &text);

private:
    mLineEdit(mLineEdit const&);
    void operator =(mLineEdit const&);
};

#endif // MLINEEDIT_H
