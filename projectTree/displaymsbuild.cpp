#include "displaymsbuild.h"
#include "msbuildthread.h"
#include <QDir>
#include <QDebug>

DisplayMSBuild::DisplayMSBuild(int total, bool procMode):QDialog(), browser(this), projectNum(total), doneNum(0)
{
    if (!procMode) {
        bar = new QProgressBar(this);
        taskStatus = showProgress;
    }
    else {
        bar = NULL;
        taskStatus = showSum;
    }

    this->resize(500, 400);
    browser.setGeometry(0,0, 500,370);

    bar->setGeometry(20, 375, 400, 20);
    if (total) {
        bar->setValue(0);
        bar->setRange(0, projectNum);
    }
}


DisplayMSBuild::~DisplayMSBuild()
{
    if (bar) {
        delete bar;
    }
}

void DisplayMSBuild::setTotalNum(int num)
{
    projectNum = num;
    doneNum = 0;
    bar->setRange(0, projectNum);
    bar->setValue(0);
}

void DisplayMSBuild::viewSumLog()
{
    QString msg;
    QDir a(QDir::current());
    QFileInfoList filelist = a.entryInfoList();
    foreach(QFileInfo file, filelist) {
        if (file.fileName().contains(ERRORLOGPATH, Qt::CaseInsensitive)) {
            msg = file.path() +"/"+file.fileName();
            if (!file.size()) {
                msg.clear();
            }
        }
    }

    if (msg.isEmpty()) {
        msg = (QDir::currentPath()+QString("/")+QString(SUMLOGPATH));
    }

    QFile f(msg);
    if(f.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&f);
        browser.setDocument(new QTextDocument(in.readAll()));
    }
}

void DisplayMSBuild::clean()
{
    browser.clear();
}

void DisplayMSBuild::handleMSBuildMsg(MSBuildThread *msbuild)
{
    if (taskStatus == showErr)
        return;

    QString msg = QString::fromLocal8Bit(msbuild->readOutPut());
    browser.append(msg);

    QTextCursor t = browser.textCursor();
    t.movePosition(QTextCursor::End);
    browser.setTextCursor(t);
}

void DisplayMSBuild::handldMSBuildFinish(int level, QString path, int exitCode)
{
    Q_UNUSED(level);
    Q_UNUSED(path);

    bar->setValue(++doneNum);

    if (exitCode) {
        qDebug() << "Disp  exit code " << exitCode;
        taskStatus = showErr;
        QString msg = (QDir::currentPath()+QString("/")+QString(ERRORLOGPATH));

        QFile f(msg);
        if(f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&f);
            browser.setDocument(new QTextDocument(in.readAll()));
        }
    }
}


