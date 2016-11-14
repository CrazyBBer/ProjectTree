#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QListWidget>
#include <QLineEdit>
#include <QDebug>

#include "mcfgdlg.h"
#include "mscene.h"
#include "praserProjFile/include/config.h"
#include "praserProjFile/include/praserSLNFile.h"
#include "msbuildthread.h"
#include "displaymsbuild.h"

#define CACHEDIRNAME        "CaChe"
#define GRAPHFILEPOSTFIX    ".grp"
#define PROJINFOFILEPOSTFIX ".rst"
#define PROJLEVELFILEPOSTFIX ".lvl"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void resetScene();
    void saveSceneItem2File();
    void saveSceneFileAs();
    void openSceneItemFile();
    void cleanCache();
    void about();

    void newConfig();
    void openConfig();
    void procConfig();

    void saveResult();
    void openResult();

    void DisplayProjList();
    void refreshProjList(QString text);
    void refreshScene(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QGraphicsView   *view;
    mScene          *s;
    mCfgDlg         *cfgDlg;
    QListWidget     *tabProjs;
    QLineEdit       *searchBox;
    MSBuildTask     *buildTask;

    enum {
        none = 0,
        ProcProjFile,
        Building
    } taskStatus;

private:
    xmap	gProjDependMap;
    slist	gProjList;
    slist	gServiceList;
    slist	g3rdLibList;
    slist	gBasicHUSLibList;

    projMap_t       projInfo;
    QString         curSceneName;
    QString         cachePath;

    int             maxLevel;
    int            *SceneItemNumatLevel;

    void dumpGlobalProjRelation();
    void dumpGlobalServiceRelation();
    bool addChielRelation(TiXmlElement * parent, QString &parentName);
    void dump3rdlibrary();
    void dumpBasicHUSLibrary();

    void paintItem(const QString name, bool core = false);
    void save2CacheFile();

    projItem *ProjInfoFind(const QString name);
    void CheckProjName(praser &p);
    void resetProjInfo();
    void procProjFile(praser &p);
    void procSLNFile(QString &file);
    void cutProjRelate();
    void updateProjLevel();
    void updateChildProjLevel(projItem *proj);
    int checkRepeat(projItem *proj, projItemList &projList, bool samelevel);

    projectArray_t * getProjByLevel();


signals:
    void updateStatus(int row, bool flag);
private slots:
    void on_actionBuild_triggered();
    void buildTaskComplete(bool status);
    void on_actionViewLog_triggered();
    void on_actionExportLevel_triggered();
};

#endif // MAINWINDOW_H
