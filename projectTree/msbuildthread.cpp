#include "msbuildthread.h"
#include "displaymsbuild.h"
#include <QDir>
#include <QDebug>

void MSBuildThread::SetProgram(QString &p)
{
    path = p;
}

void MSBuildThread::SetProject(QString &p)
{
    project = p;
}

void MSBuildThread::SetTarget(QString &t)
{
    target = QString("/t:") + t;
}

void MSBuildThread::SetLevel(int l)
{
    level = l;
}

void MSBuildThread::run()
{
    errMsg = QString("Ready 2 Run");
    msbuild = new QProcess();
    if (!msbuild) {
        errMsg.push_back("Create QProcess failed");
        return;
    }

    connect(msbuild, &QProcess::readyReadStandardOutput, this, &MSBuildThread::hanldTaskReadOutPutSingle);

    msbuild->setProgram(path);

    QStringList args;
    args << project << target;      //Verbosity=minimal;
    args << "/p:Configuration=Release"<<"/m"
            << QString("/flp:Summary;append;LogFile=")+QString(SUMLOGPATH)
            << QString("/flp1:warningsonly;append;logfile=") + QString(WARNLOGPATH)
            << QString("/flp2:errorsonly;append;logfile=")+QString(ERRORLOGPATH);

    msbuild->setArguments(args);

    msbuild->start();
    if (!msbuild->waitForStarted()) {
        errMsg.push_back( "StartFailed");
        emit taskProcessStatus(this);
        return ;
    }

    if (!msbuild->waitForFinished()) {
        errMsg.push_back("FinishedFailed");
        emit taskProcessStatus(this);
        return;
    }

    errMsg.push_back("Task finished");
    emit almostFinished(level, path, msbuild->exitCode());
}

QByteArray MSBuildThread::readOutPut()
{
    if (!msbuild)
        return QByteArray();
    return msbuild->readAllStandardOutput();
}

QString MSBuildThread::readErrMsg()
{
    return errMsg;
}

void MSBuildThread::hanldTaskReadOutPutSingle()
{
    emit ready2ReadOutput(this);
}


MSBuildTask::MSBuildTask():size(0), display(NULL), projectListArray(NULL)
{
}


MSBuildTask::~MSBuildTask()
{
    if (projectListArray) {
        delete [] projectListArray;
    }
}

bool MSBuildTask::setDisplayDialog(int projNum)
{
    if (!display) {
        display = new DisplayMSBuild(projNum);
        if (!display) {
            return false;
        }
    } else if (projNum) {
        display->setTotalNum(projNum);//set num;
    }

    display->viewSumLog();

    display->show();

    return true;
}

bool MSBuildTask::setTask(int s, projectArray_t *projSortByLevel)
{
    if (taskStatus == started)
        return false;

    if (s <= 0 || !projSortByLevel)
        return false;

    size = s;
    if (projectListArray) {
        MSBuildTask::cleanProjectArray(projectListArray);
        projectListArray = NULL;
    }

    projectListArray = projSortByLevel;

    return true;
}

bool MSBuildTask::startTask()
{
    if (taskStatus == started)
        return true;

    if (!display || !projectListArray) {
        return false;
    }

    if (taskStatus != exiting) {
        taskStatus = started;

        QDir a(QDir::current());
        a.remove(QString(SUMLOGPATH));
        a.remove(QString(ERRORLOGPATH));
        a.remove(QString(WARNLOGPATH));

        display->clean();

        callMSBuild(0);
    }

    return true;
}

void MSBuildTask::cancelTask()
{
    taskStatus = exiting;

}

void MSBuildTask::cleanProjectArray(projectArray_t *array)
{
    if (!array)
        return;

    while(array->size()) {
        QStringList *list = *array->begin();
        array->pop_front();
        delete list;
    }
    delete array;
    array = NULL;
}

void MSBuildTask::callMSBuild(int level)
{
    if (level >= size) {
        emit taskComplete(true);
        taskStatus = none;
        return;
    } else if (level < 0) {
        emit taskComplete(false);
        taskStatus = none;
        return;
    }

    QStringList *list = projectListArray->at(level);

    QString path = *(list->begin());
    if (list->size() == 0 || path.isEmpty()) {
        list->pop_front();
        msbuildThreadFinished(level, path, 0);
        display->handldMSBuildFinish(level, path, 0);
        return;
    }

    MSBuildThread *msbuild = new MSBuildThread();
    if (!msbuild)
        return;

    msbuild->SetProgram(QString(MSBUILDPATH));
    msbuild->SetProject(*(list->begin()));
    list->pop_front();
    msbuild->SetTarget(QString("Build"));
    msbuild->SetLevel(level);

    connect(msbuild, &MSBuildThread::ready2ReadOutput, display, &DisplayMSBuild::handleMSBuildMsg);
    connect(msbuild, &MSBuildThread::almostFinished, display, &DisplayMSBuild::handldMSBuildFinish);
    connect(msbuild, &MSBuildThread::almostFinished, this, &MSBuildTask::msbuildThreadFinished);
    connect(msbuild, &MSBuildThread::finished, msbuild, &MSBuildThread::deleteLater);

    msbuild->start();
}

void MSBuildTask::msbuildThreadFinished(int level, QString path, int exitCode)
{
    if (level >= size || level < 0)
        return;

    QStringList *list = projectListArray->at(level);

    if (exitCode) {
        taskStatus = exiting;
    }

    int NextLevel = level;
    if (list->size() == 0) {
        NextLevel = level+1;
    }
    if (taskStatus == exiting) {
        NextLevel = -1;
    }

    callMSBuild(NextLevel);
}

