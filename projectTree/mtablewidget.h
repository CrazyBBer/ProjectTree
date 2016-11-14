#ifndef MTABLEWIDGET_H
#define MTABLEWIDGET_H

#include <QTableWidget>

class mTableWidget : public QTableWidget
{
public:
    mTableWidget(QWidget *parent = 0);
public slots:
    void updateStatus(int row, bool flag);
};

#endif // MTABLEWIDGET_H
