#include "mtablewidget.h"

mTableWidget::mTableWidget(QWidget *parent):QTableWidget(parent)
{

}

void mTableWidget::updateStatus(int row, bool flag)
{
    if (flag == false){
        QTableWidgetItem *status = item(row,1);
        if (!status) {
            status = new QTableWidgetItem(QString("doing"));
            setItem(row,1, status);
        }
        status->setText(QString("doing"));
        status->setTextColor(Qt::gray);
    } else {
        QTableWidgetItem *status = item(row,1);
        if (!status) {
            status = new QTableWidgetItem(QString("done"));
            setItem(row,1, status);
        }
        status->setText(QString("done"));
        status->setTextColor(Qt::green);
    }
}
