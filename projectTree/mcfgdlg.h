#ifndef MCFGDLG_H
#define MCFGDLG_H

#include <QDialog>
#include <QTableWidget>
#include <QMap>
#include "projitem.h"
#include "mtablewidget.h"
#include "praserProjFile/include/praserSLNFile.h"
#include "praserProjFile/include/praserCSprojFile.h"
#include "praserProjFile/include/praserVCprojFile.h"

namespace Ui {
class Dialog;
}

typedef QMap<QString, projItem*> projMap_t;


class QPushButton;

class mCfgDlg : public QDialog
{
    Q_OBJECT

public:
    mCfgDlg(QWidget *parent = 0);
    void clear();
    bool openCfg(QString &fileName);


    QPushButton *btnProc;
    mTableWidget *mtab;

public slots:
    void addProjFile();
    void delProjFile();
    void saveConfigFile(bool flag);
    void saveConfigFileAs();

private slots:
    void on_saveResult_clicked();

private:
    bool isexist(QString &filename);

    Ui::Dialog *ui;
    QString curFileName;

signals:
    void SaveResultSIG();
};

#endif // MCFGDLG_H
