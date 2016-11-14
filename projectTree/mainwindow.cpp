#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QGraphicsItem>
#include <mTextItem.h>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),projInfo(), maxLevel(0), SceneItemNumatLevel(NULL), taskStatus(none)
{
    ui->setupUi(this);
    cfgDlg = NULL;

    Qt::WindowFlags flags = 0;
    flags |= Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
    setFixedSize(1000,700);

    s = new mScene();

    view = new QGraphicsView(s, this);
    view->setAcceptDrops(true);
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    view->setGeometry(QRect(QPoint(150,22), QSize(850, 660)));

    tabProjs = new QListWidget(this);
    searchBox = new QLineEdit(this);
    QLabel *searchText = new QLabel(QString("Search Box:"),this);
    searchText->setGeometry(QRect(QPoint(1,25), QSize(100, 25)));
    searchBox->setGeometry(QRect(QPoint(1,50), QSize(145,25)));
    QLabel *ProjectListText = new QLabel(QString("Project List"),this);
    ProjectListText->setGeometry(QRect(QPoint(1,75), QSize(100, 25)));
    tabProjs->setGeometry(QRect(QPoint(1,100), QSize(150,580)));
    buildTask = new MSBuildTask();
    ui->statusBar->showMessage(QString("Load UI Done"));

    if (!cfgDlg)
        cfgDlg = new mCfgDlg(this);

    cachePath = QDir::currentPath() + QString('/') + QString(CACHEDIRNAME);
    if (!QDir(cachePath).exists())
        QDir().mkdir(cachePath);

    curSceneName = QDir::currentPath() + QString('/')\
            + QString("lastGraph") + QString(GRAPHFILEPOSTFIX);

    //SIGNAL
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::resetScene);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openSceneItemFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveSceneItem2File);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveSceneFileAs);
    connect(ui->actionOpenConfig, &QAction::triggered, this, &MainWindow::openConfig);
    connect(ui->actionNewConfig, &QAction::triggered, this, &MainWindow::newConfig);
    connect(ui->actionClean, &QAction::triggered, this, &MainWindow::cleanCache);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);

    if (cfgDlg) {
        //connect(cfgDlg, &mCfgDlg::procComplete, this, &MainWindow::DisplayProjList);
        connect(cfgDlg->btnProc, &QPushButton::clicked, this, &MainWindow::procConfig);
        connect(this, &MainWindow::updateStatus, cfgDlg->mtab, &mTableWidget::updateStatus);

        connect(cfgDlg, &mCfgDlg::SaveResultSIG, this, &MainWindow::saveResult);
    }

    connect(tabProjs, &QListWidget::itemDoubleClicked, this, &MainWindow::refreshScene);
    connect(searchBox, &QLineEdit::textEdited, this, &MainWindow::refreshProjList);
    connect(ui->actionSaveResult, &QAction::triggered, this, &MainWindow::saveResult);
    connect(ui->actionOpenResult, &QAction::triggered, this, &MainWindow::openResult);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::about()
{
    QDialog about(this);
    about.resize(200, 100);
    QLabel *version = new QLabel(QString("Version: 3.0"), &about);
    version->setGeometry(QRect(QPoint(50, 35), QSize(100, 25)));
    about.exec();
}

void MainWindow::dumpGlobalProjRelation()
{
    QString fileName = PROJRELEATE + QString("\\") + "ALLProject.Relation.xml";
    TiXmlDocument doc;

    TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "", "");
    TiXmlElement *slnElement = new TiXmlElement("AllSLNRelation");

    for (sit si = gProjList.begin(); si != gProjList.end(); si++) {
        TiXmlElement *projEle = new TiXmlElement("Project");
        projEle->SetAttribute("TargetName", (*si).toLatin1().data());
        xmit xi = gProjDependMap.find(*si);
        if (xi != gProjDependMap.end()) {
            slist depList = xi.value();
            for (sit s = depList.begin(); s != depList.end(); s++) {
                TiXmlElement *relation = new TiXmlElement("Relation");
                TiXmlText *text = new TiXmlText((*s).toLatin1().data());
                relation->LinkEndChild(text);
                projEle->LinkEndChild(relation);
            }
        }

        slnElement->LinkEndChild(projEle);
    }

    doc.LinkEndChild(dec);
    doc.LinkEndChild(slnElement);
    doc.SaveFile(fileName.toLatin1().data());

    return;
}


bool MainWindow::addChielRelation(TiXmlElement * parent, QString &parentName)
{
    parent->SetAttribute("Name", parentName.toLatin1().data());
    xmit xi = gProjDependMap.find(parentName);
    if (xi == gProjDependMap.end()) {
        return false;
    }

    slist depList = xi.value();
    if (depList.empty()) {
        return false;
    }

    for (sit s = depList.begin(); s != depList.end(); s++) {
        TiXmlElement *relation = new TiXmlElement("Relation");

        addChielRelation(relation, *s);

        parent->LinkEndChild(relation);
    }
    return true;
}


void MainWindow::cleanCache()
{
    QDir cacheDir(QDir::currentPath()+QString("/")+QString(CACHEDIRNAME));
    QStringList filter("*.grp");
    QStringList fileList = cacheDir.entryList(filter);
    foreach(QString file, fileList) {
        cacheDir.remove(file);
    }
}

void MainWindow::dumpGlobalServiceRelation()
{
    QString fileName = PROJRELEATE + QString("\\") + "ALLServices.Relation.xml";
    TiXmlDocument doc;

    TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "", "");
    TiXmlElement *slnElement = new TiXmlElement("AllServicesRelation");

    for (sit si = gServiceList.begin(); si != gServiceList.end(); si++) {
        TiXmlElement *ele = new TiXmlElement("Services");
        addChielRelation(ele, *si);
        slnElement->LinkEndChild(ele);
    }

    doc.LinkEndChild(dec);
    doc.LinkEndChild(slnElement);
    doc.SaveFile(fileName.toLatin1().data());

    return ;
}

void MainWindow::dump3rdlibrary()
{
    for (xmit xi = gProjDependMap.begin(); xi != gProjDependMap.end(); xi++) {
        slist dependencList = xi.value();
        for (sit si = dependencList.begin(); si != dependencList.end(); si++) {
            // log4cpp.lib ==> log4cpp  log4cpp.dll ==> log4cpp
            if (caseCmp(gProjList, *si) == false) {
                if (caseCmp(g3rdLibList, *si) == false)
                    g3rdLibList.push_back(*si);
            }
        }
    }

    QString fileName = PROJRELEATE + QString("\\") + "ALL3rdPartLibrary.xml";
    TiXmlDocument doc;

    TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "", "");
    TiXmlElement *slnElement = new TiXmlElement("ThridPartLibrary");

    for (sit si = g3rdLibList.begin(); si != g3rdLibList.end(); si++) {
        TiXmlElement *ele = new TiXmlElement("Library");
        TiXmlText *text = new TiXmlText((*si).toLatin1().data());

        ele->LinkEndChild(text);
        slnElement->LinkEndChild(ele);
    }

    doc.LinkEndChild(dec);
    doc.LinkEndChild(slnElement);
    doc.SaveFile(fileName.toLatin1().data());

    return ;
}


void MainWindow::dumpBasicHUSLibrary()
{
    for (sit si = gProjList.begin(); si != gProjList.end(); si++) {
        xmit xi= gProjDependMap.find(*si);
        if (xi != gProjDependMap.end()) {	// no dependence
            if (xi.value().empty())	// no dependence
                if (caseCmp(g3rdLibList, *si) == false)	// not 3rd project
                    if ((*si).contains(".exe") == false && nStrCmp((*si).toLatin1().data(), "Test")!=0) {
                        gBasicHUSLibList.push_back(*si);
                    }
        } else {
            ;//error condition
        }
    }

    QString fileName = PROJRELEATE + QString("\\") + "ALLHUSBasicLib.xml";
    TiXmlDocument doc;

    TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "", "");
    TiXmlElement *slnElement = new TiXmlElement("HUSBasicLibrary");

    for (sit si = gBasicHUSLibList.begin(); si != gBasicHUSLibList.end(); si++) {
        TiXmlElement *ele = new TiXmlElement("Library");
        TiXmlText *text = new TiXmlText((*si).toLatin1().data());

        ele->LinkEndChild(text);
        slnElement->LinkEndChild(ele);
    }

    doc.LinkEndChild(dec);
    doc.LinkEndChild(slnElement);
    doc.SaveFile(fileName.toLatin1().data());

    return ;
}


void MainWindow::procConfig()
{
    if (taskStatus != none) {
        ui->statusBar->showMessage("Task is busy now!");
        return;
    }
    taskStatus = ProcProjFile;

    resetProjInfo();

    for (int i = 0; i < cfgDlg->mtab->rowCount(); i++) {
        emit updateStatus(i, false);

        QString file = cfgDlg->mtab->item(i,0)->text();

        if (file.right(4) == ".sln")
            procSLNFile(file);
        else if (file.right(7) == ".csproj") {
            csProjPrase p(file);
            if (p.Do() == 0) {
                procProjFile(p);
            }
        }
        else if (file.right(8) == ".vcxproj"){
            vcProjPrase p(file);
            if (p.Do() == 0) {
                procProjFile(p);
            }
        }
        else
            qDebug() << "Unknown file type 2: " << file;

        emit updateStatus(i, true);
        ui->statusBar->showMessage(QString("Process: ")+file);
    }
    ui->statusBar->showMessage(QString("Process: Done"));

    cutProjRelate();

    updateProjLevel();

    DisplayProjList();

    taskStatus = none;
}


void MainWindow::resetProjInfo()
{
    projMap_t::iterator it = projInfo.begin();
    while ( it != projInfo.end()) {
        projItem *proj = it.value();
        projInfo.erase(it);

        delete proj;

        it = projInfo.begin();
    }
}

void MainWindow::CheckProjName(praser &p)
{
    if (p.projName.contains("$(TargetExt)")) {
        p.projName.remove("$(TargetExt)");
    }

    if (p.projName.contains("$(TargetName)")) {
        p.projName.replace("$(TargetName)", p.file);
    }

    if (p.projName.contains("_ZH-CHS")) {
        p.projName.remove("_ZH-CHS");
    }
    if (p.projName.contains("_KO-KR")) {
        p.projName.remove("_KO-KR");
    }
    if (p.projName.contains("_ZH-CHT")) {
        p.projName.remove("_ZH-CHT");
    }
    if (p.projName.contains("_EN-US")) {
        p.projName.remove("_EN-US");
    }
}

projItem *MainWindow::ProjInfoFind(const QString name)
{
    for (projMap_t::iterator it = projInfo.begin();
         it != projInfo.end();
         it++) {
        if (QString::compare(it.key(), name, Qt::CaseInsensitive)==0) {
            return it.value();
        }
    }

    return NULL;
}


void MainWindow::procProjFile(praser &p)
{
    /*
    if (p.projName.contains("sscon", Qt::CaseInsensitive)) {
        qDebug() << p.projName << p.projPath;
    }*/

    CheckProjName(p);


    projItem *it  = ProjInfoFind(p.projName);
    if (!it)
        it = new projItem(p.projName, p.projPath);

    if (!it) {
        return;
    }

    if (it->path.isEmpty()) {
        it->path = p.projPath;
    }

    if (p.projPostFix == ".dll")
        it->type = projItem::kDLL;
    else if (p.projPostFix == ".lib")
        it->type = projItem::kLIB;
    else if (p.projPostFix == ".exe")
        it->type = projItem::kEXE;
    else
        it->type = projItem::kUnknown;

    foreach (QString proj, p.projDepend) {
        projItem *chit =ProjInfoFind(proj);
        if (!chit) {
            chit = new projItem(proj);
            if (!chit) {
                return;
            }
            projInfo.insert(proj, chit);
        }

        if (chit->parents.contains(it) == false) {
            chit->parents.push_back(it);
        }

        if (it->chilrens.contains(chit) == false) {
            it->chilrens.push_back(chit);
        }
    }

    if (projInfo.contains(p.projName) == false)
        projInfo.insert(p.projName, it);
}


void MainWindow::procSLNFile(QString &file)
{
    slnPrase slnFile(file);
    QStringList projList;
    projList.clear();
    slnFile.loadSLNFile(projList);

    foreach (QString proj, projList) {
        if (proj.right(7) == ".csproj") {
            csProjPrase p(proj);
            int ret = p.Do();
            if (ret == 0) {
                procProjFile(p);
            } else if(ret == -1) {
                qDebug() <<ret<< " Do failed: "<<proj;
            }
        }
        else if (proj.right(8) == ".vcxproj"){
            vcProjPrase p(proj);
            int ret = p.Do();
            if (ret == 0) {
                procProjFile(p);
            } else if(ret == -1) {
                qDebug() <<ret<< " Do failed: "<<proj;
            }
        }
        else
            qDebug() << "Unknown file type 4: " << file;
    }
}

void MainWindow::cutProjRelate()
{
    for(projMap_t::iterator it = projInfo.begin(); it != projInfo.end(); it++) {
        projItem *proj = it.value();

        projItemList::iterator lastIt = proj->chilrens.end();
        int repeatFlag = 0;

        for(projItemList::iterator childIt = proj->chilrens.begin();
            childIt != proj->chilrens.end();) {

            if (repeatFlag) {
                repeatFlag = false;

                if (lastIt != proj->chilrens.end())
                    childIt = lastIt+1;
                else
                    childIt = proj->chilrens.begin();

            }

            if ((*childIt)->level <= proj->level)
                (*childIt)->level = proj->level + 1;

            repeatFlag = checkRepeat(*childIt, proj->chilrens, true);
            if (repeatFlag == 1) {
                (*childIt)->parents.removeOne(proj);
                proj->chilrens.removeOne(*childIt);
                continue;
            } else if (repeatFlag == -1) {
                qDebug() << "Head:"<<*childIt;
                break;
            }

            lastIt = childIt;
            childIt++;
        }
    }
}


int MainWindow::checkRepeat(projItem *proj, projItemList &projList, bool samelevel)
{
    foreach(projItem *eachProj, projList) {
        if (eachProj->flag == true) {
            qDebug() << "Error: cycle releate! "<< eachProj;
            return -1;
        }
        if (eachProj == proj) {
            if (samelevel) {
                continue;
            } else {
                return 1;
            }
        }

        eachProj->flag = true;
        int repeat = checkRepeat(proj, eachProj->chilrens, false);
        eachProj->flag = false;

        if (repeat == 1)
            return 1;
        else if (repeat == -1) {
            qDebug() << "   "<<eachProj;
            return -1;
        }

    }

    return 0;
}

projectArray_t *MainWindow::getProjByLevel()
{
    projectArray_t *projSortByLevel = new projectArray_t();
    if (!projSortByLevel) {
        return NULL;
    }
    for (int i = 0; i < maxLevel +1; i++) {
        QStringList *list = new QStringList();
        if (!list) {
            MSBuildTask::cleanProjectArray(projSortByLevel);
            taskStatus = none;
            return NULL;
        }

        projSortByLevel->push_back(list);
    }

    for(projMap_t::iterator it = projInfo.begin(); it != projInfo.end(); it++) {
        projItem *proj = it.value();
        if (proj->path.length() && !projSortByLevel->at(proj->level)->contains(proj->path)) {
            projSortByLevel->at(proj->level)->push_back(proj->path);
        }
    }

    return projSortByLevel;
}


void MainWindow::updateChildProjLevel(projItem *proj)
{
    foreach(projItem *p, proj->chilrens) {
        if (proj->level >= p->level)
            p->level = proj->level +1;

        if (maxLevel < p->level)
            maxLevel = p->level;

        updateChildProjLevel(p);
    }
}


void MainWindow::updateProjLevel()
{
    for (projMap_t::iterator it = projInfo.begin(); it != projInfo.end(); it++) {
        projItem *proj = it.value();
        int projLevel = 0;
        foreach(projItem *p, proj->parents) {
            if (projLevel <= p->level)
                projLevel++;
        }
        if (proj->level < projLevel)
            proj->level = projLevel;

        if (maxLevel < proj->level)
            maxLevel = proj->level;

        updateChildProjLevel(proj);
    }
}

void MainWindow::paintItem(const QString name, bool core)
{
    projItem* proj = ProjInfoFind(name);
    if (!proj) {
        ui->statusBar->showMessage(QString("Not find this project info: ")+name);
        return;
    }

    int c;
    if (core)
        c = Qt::lightGray;
    else
        c = Qt::white;
    if (proj->type == projItem::kEXE)
        c = Qt::green;
    if (proj->path.isEmpty())
        c = Qt::yellow;

    mTextItem *first = new mTextItem(proj->name, (Qt::GlobalColor)c);
    if (!s->addmTextItem(first)) {
        delete first;
        return;
    }

    first->setPos(0 + 100 * (SceneItemNumatLevel[proj->level]++), 100*proj->level);

    {
        // add child list
        foreach(projItem* childProj, proj->chilrens) {
            s->addLinkedItem(first, childProj, mScene::eChildItem, SceneItemNumatLevel);
        }
    }
    {
        // add parent list
        foreach(projItem* parentProj, proj->parents) {
            s->addLinkedItem(first, parentProj, mScene::eParentItem, SceneItemNumatLevel);
        }
    }
    view->centerOn(first);
    return ;
}

void MainWindow::resetScene()
{
    save2CacheFile();

    s->sceneReset();

    if (SceneItemNumatLevel) {
        delete [] SceneItemNumatLevel;
        SceneItemNumatLevel = NULL;
    }

    if (!SceneItemNumatLevel) {
        int bufLen = maxLevel +1;
        SceneItemNumatLevel = new int[bufLen];
        for (int i = 0; i < bufLen; i++) {
            SceneItemNumatLevel[i] = 0;

            //BUG??
//            if (SceneItemNumatLevel) {
//                memset(SceneItemNumatLevel, 0, bufLen);
//            }
        }
    }

}

void MainWindow::saveSceneItem2File()
{
    //if (s->getCurFileName().isEmpty())
        saveSceneFileAs();
    //else
        //s->save2file(s->getCurFileName());

    ui->statusBar->showMessage(QString( "saveSceneItem2File"));
}


void MainWindow::saveSceneFileAs()
{
    QFileDialog *f = new QFileDialog(this);
    f->setAcceptMode(QFileDialog::AcceptSave);
    f->setNameFilter(QString("*.grp"));
    QStringList fileNames;
    if (f->exec()) {
        fileNames = f->selectedFiles();
    }
    if(fileNames.isEmpty()){
        return;
    }

    QString file = fileNames.at(0);
    if (file.right(4) != QString(GRAPHFILEPOSTFIX))
        file += GRAPHFILEPOSTFIX;

    s->save2file(file);

    save2CacheFile();

    ui->statusBar->showMessage(QString( "saveSceneItem2File"));
}


void MainWindow::openSceneItemFile()
{
    QFileDialog *f = new QFileDialog(this);
    f->setNameFilter(QString("*.grp"));
    f->setFileMode(QFileDialog::ExistingFile);
    QStringList fileNames;
    if (f->exec()) {
        fileNames = f->selectedFiles();
    }

    if (fileNames.isEmpty())
        return;

    s->loadfromfile(fileNames.at(0));
    ui->statusBar->showMessage(QString( "openSceneItemFile"));
}

void MainWindow::openConfig()
{
    QFileDialog *f = new QFileDialog(this);
    f->setAcceptMode(QFileDialog::AcceptOpen);
    f->setNameFilter(QString("*.cfg"));
    QStringList fileNames;
    if (f->exec()) {
        fileNames = f->selectedFiles();
    }
    if(fileNames.isEmpty()){
        return;
    }

    QString file = fileNames.at(0);
    if (file.right(4) != QString(".cfg"))
        file += ".cfg";

    if (!cfgDlg) {
        cfgDlg = new mCfgDlg(this);
    }
    cfgDlg->openCfg(file);
    cfgDlg->show();
}


void MainWindow::newConfig()
{
    if (!cfgDlg) {
        cfgDlg = new mCfgDlg(this);
    }
    cfgDlg->clear();
    cfgDlg->show();
}

void MainWindow::DisplayProjList()
{
    QStringList projList;
    tabProjs->clear();
    for(projMap_t::iterator it = projInfo.begin();
        it != projInfo.end(); it++) {
        if (projList.contains(it.value()->name) == false) {
            QString postFix;
            if (it.value()->type == projItem::kDLL)
                postFix = QString(".dll");
            else if (it.value()->type == projItem::kEXE)
                postFix = QString(".exe");
            else if (it.value()->type == projItem::kLIB)
                postFix = QString(".lib");

            projList.push_back(it.value()->name+postFix);
        }
    }
    tabProjs->addItems(projList);
}

void MainWindow::refreshScene(QListWidgetItem *item)
{
    resetScene();

    QString proj;
    if (item->text().right(4) == ".dll" ||
            item->text().right(4) == ".lib" ||
            item->text().right(4) == ".exe") {
        proj = item->text().left(item->text().lastIndexOf('.'));
    } else {
        proj = item->text();
    }

    projItem *DisplayProj = ProjInfoFind(proj);
    if (!DisplayProj) {
        ui->statusBar->showMessage(QString("Not find this project info: ")+proj);
        return;
    }
    ui->statusBar->showMessage(DisplayProj->path);

    QString cacheFileName = cachePath + QString('/') + proj + QString(GRAPHFILEPOSTFIX);
    curSceneName = cacheFileName;
    if (QFile(cacheFileName).exists()) {
        s->loadfromfile(cacheFileName);
        return;
    } else {
        s->sceneReset();
    }

    paintItem(proj, true);
}

void MainWindow::refreshProjList(QString text)
{
    if (text.isEmpty())
        return DisplayProjList();

    QStringList displayList;
    QList<QListWidgetItem*> keyList = tabProjs->findItems(text, Qt::MatchContains);

    if (!keyList.isEmpty()) {
        foreach(QListWidgetItem *key, keyList) {
            displayList.push_back(key->text());
        }

        tabProjs->clear();
        tabProjs->addItems(displayList);
    }
}

void MainWindow::save2CacheFile()
{
    s->save2file(curSceneName);
}

void MainWindow::saveResult()
{
    if(projInfo.isEmpty()) {
        return;
    }

    QFileDialog *f = new QFileDialog(this);
    f->setAcceptMode(QFileDialog::AcceptSave);
    f->setNameFilter(QString("*.rst"));
    QStringList fileNames;
    if (f->exec()) {
        fileNames = f->selectedFiles();
    }
    if(fileNames.isEmpty()){
        return;
    }

    QString file = fileNames.at(0);
    if (file.right(4) != QString(PROJINFOFILEPOSTFIX))
        file += PROJINFOFILEPOSTFIX;

    TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "", "");
    TiXmlElement *listElement = new TiXmlElement("ProjectInfomation");

    for (projMap_t::iterator it = projInfo.begin(); it != projInfo.end(); it++) {
        projItem;
        TiXmlElement *itele = new TiXmlElement("Iterator");
        itele->SetAttribute("Key", it.key().toLatin1().data());

        TiXmlElement *projele = new TiXmlElement("Value");
        projele->SetAttribute("Type", it.value()->type);
        projele->SetAttribute("Level", it.value()->level);
        projele->SetAttribute("Name", it.value()->name.toLatin1().data());
        projele->SetAttribute("Path", it.value()->path.toLatin1().data());

        TiXmlElement *relaParentele = new TiXmlElement("RelateParents");
        foreach (projItem* p, it.value()->parents) {
            TiXmlElement *proj = new TiXmlElement("Project");
            proj->SetAttribute("ProjName", p->name.toLatin1().data());
            relaParentele->LinkEndChild(proj);
        }

        TiXmlElement *relaChildele = new TiXmlElement("RelateChildrens");
        foreach (projItem* p, it.value()->chilrens) {
            TiXmlElement *proj = new TiXmlElement("Project");
            proj->SetAttribute("ProjName", p->name.toLatin1().data());
            relaChildele->LinkEndChild(proj);
        }

        projele->LinkEndChild(relaParentele);
        projele->LinkEndChild(relaChildele);

        itele->LinkEndChild(projele);
        listElement->LinkEndChild(itele);
    }

    TiXmlDocument doc;
    doc.LinkEndChild(dec);
    doc.LinkEndChild(listElement);
    doc.SaveFile(file.toLatin1().data());
}

void MainWindow::openResult()
{
    if (taskStatus != none) {
        ui->statusBar->showMessage("Task is busy now!");
        return;
    }

    QFileDialog *f = new QFileDialog(this);
    f->setAcceptMode(QFileDialog::AcceptOpen);
    f->setNameFilter(QString("*.rst"));
    QStringList fileNames;
    if (f->exec()) {
        fileNames = f->selectedFiles();
    }
    if(fileNames.isEmpty()){
        return;
    }

    QString file = fileNames.at(0);
    if (file.right(4) != QString(PROJINFOFILEPOSTFIX))
        file += PROJINFOFILEPOSTFIX;

    TiXmlDocument doc(file.toLatin1().data());
    if (doc.LoadFile() == false) {
        return;
    }

    TiXmlNode* root = doc.FirstChild("ProjectInfomation");
    if (!root) {
        return;
    }

    taskStatus = ProcProjFile;
    resetProjInfo();

    for (TiXmlElement *ele = root->FirstChildElement("Iterator");
         ele; ele = ele->NextSiblingElement("Iterator")){
        QString key = ele->Attribute("Key");
        if (projInfo.contains(key))
            continue;
        TiXmlElement *proj = ele->FirstChildElement("Value");
        if (!proj) {
            qDebug() << "Load key but not found projInfo: " << key;
            continue;
        }
        QString name = proj->Attribute("Name");
        QString path = proj->Attribute("Path");

        projItem *projIt = new projItem(name, path);
        if (!projIt) {
            qDebug() << "Load ProjectInfo Failed, when new projItem";
            continue;
        }

        int type, level;
        proj->Attribute("Type", &type);
        proj->Attribute("Level", &level);
        projIt->type = type;
        projIt->level = level;

        if (maxLevel < level)
            maxLevel = level;

        projInfo.insert(key, projIt);
    }

    for (TiXmlElement *ele = root->FirstChildElement("Iterator");
         ele; ele = ele->NextSiblingElement("Iterator")){
        QString key = ele->Attribute("Key");
        if (!projInfo.contains(key))
            continue;

        projItem *projIt = projInfo[key];
        if (!projIt) {
            qDebug() << "Open Result Not Find ProjIt: key "<< key;
            continue;
        }

        TiXmlElement *proj = ele->FirstChildElement("Value");
        if (!proj) {
            qDebug() << "Load key but not found projInfo: " << key;
            continue;
        }
        TiXmlElement *relaParentele = proj->FirstChildElement("RelateParents");
        for(TiXmlElement *p = relaParentele->FirstChildElement("Project");
            p;p = p->NextSiblingElement("Project")) {
            QString name = p->Attribute("ProjName");

            projItem *parent = projInfo[name];
            if (!parent) {
                qDebug() << "Add Parent but not find project: name " << name;
                continue;
            }
            projIt->parents.push_back(parent);
        }
        TiXmlElement *relaChildele = proj->FirstChildElement("RelateChildrens");
        for(TiXmlElement *p = relaChildele->FirstChildElement("Project");
            p;p = p->NextSiblingElement("Project")) {
            QString name = p->Attribute("ProjName");

            projItem *child = projInfo[name];
            if (!child) {
                qDebug() << "Add child but not find project: name " << name;
                continue;
            }
            projIt->chilrens.push_back(child);
        }
    }
    ui->statusBar->showMessage("Open Result info done!");

    DisplayProjList();

    taskStatus = none;
}

void MainWindow::on_actionBuild_triggered()
{
    if (projInfo.isEmpty()) {
        ui->statusBar->showMessage("Not found project info");
        return;
    }

    if (taskStatus != none) {
        ui->statusBar->showMessage("Task is busy now!");
        return;
    }
    taskStatus = Building;

    //projectArray_t *projSortByLevel = new projectArray_t();
    projectArray_t *projSortByLevel = getProjByLevel();
    if (!projSortByLevel) {
        ui->statusBar->showMessage("Create Project Array failed");
        taskStatus = none;
        return;
    }

    if (!buildTask->setTask(maxLevel+1, projSortByLevel)) {
        MSBuildTask::cleanProjectArray(projSortByLevel);
        taskStatus = none;
        ui->statusBar->showMessage("BuildThread setTask failed");
        return;
    }

    if (!buildTask->setDisplayDialog(projInfo.size())) {
        MSBuildTask::cleanProjectArray(projSortByLevel);
        taskStatus = none;
        ui->statusBar->showMessage("BuildThread setDisplayDialog failed");
        return;
    }

    if (!buildTask->startTask()) {
        MSBuildTask::cleanProjectArray(projSortByLevel);
        taskStatus = none;
        ui->statusBar->showMessage("BuildThread startTask failed");
        return;
    }

    taskStatus = none;
}

void MainWindow::buildTaskComplete(bool status)
{
    Q_UNUSED(status);
    if (taskStatus == Building)
        taskStatus = none;
}

void MainWindow::on_actionViewLog_triggered()
{
    buildTask->setDisplayDialog();
}

void MainWindow::on_actionExportLevel_triggered()
{
    //Check
    if (projInfo.isEmpty())
        return;

    //GetFileName
    QFileDialog *f = new QFileDialog(this);
    f->setAcceptMode(QFileDialog::AcceptSave);
    f->setNameFilter(QString("*.lvl"));
    QStringList fileNames;
    if (f->exec()) {
        fileNames = f->selectedFiles();
    }
    if(fileNames.isEmpty()){
        return;
    }

    QString file = fileNames.at(0);
    if (file.right(4) != QString(PROJLEVELFILEPOSTFIX))
        file += PROJLEVELFILEPOSTFIX;

    //GetProjLevel
    projectArray_t *projSortByLevel = getProjByLevel();
    if (!projSortByLevel) {
        return;
    }

    //DumpFile
    TiXmlDocument doc;
    TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "", "");
    TiXmlElement *root = new TiXmlElement("ProjectLevel");

    for (int i = 0; i < projSortByLevel->size(); i++) {
        if (projSortByLevel->at(i)->isEmpty())
            continue;

        QStringList *list = projSortByLevel->at(i);

        TiXmlElement *ele = new TiXmlElement("Item");
        if (!ele)
            continue;

        ele->SetAttribute("Level", i);
        ele->SetAttribute("Count", list->size());
        foreach(QString path, *list) {
            TiXmlElement *project = new TiXmlElement("Item");
            if (!project)
                continue;
            path.replace('/', '\\');
            project->SetAttribute("Project", path.toLatin1().data());
            ele->LinkEndChild(project);
        }
        root->LinkEndChild(ele);
    }

    doc.LinkEndChild(dec);
    doc.LinkEndChild(root);
    doc.SaveFile(file.toLatin1().data());
}
