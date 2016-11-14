#ifndef MLISTWIDGET_H
#define MLISTWIDGET_H

#include <QListWidget>

class mlistWidget : public QListWidget
{
public:
    mlistWidget();

public slots:
    void MyDoubleClicked(QListWidgetItem *item);
};

#endif // MLISTWIDGET_H
