#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width()-w.width()) / 2;
    int y = (screenGeometry.height()-w.height()) / 2;
    w.move(x, y);

    w.setWindowTitle(QString("Project Tree tool"));
    w.show();

    return a.exec();
}
