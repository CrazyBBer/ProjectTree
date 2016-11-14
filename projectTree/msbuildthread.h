#ifndef MSBUILDTHREAD_H
#define MSBUILDTHREAD_H


#include <QThread>
#include <QProcess>

#define ERRORLOGPATH        "msbuild.err"
#define WARNLOGPATH         "msbuild.wrn"
#define SUMLOGPATH          "msbuild.sum"


#define MSBUILDPATH         "C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\msbuild.exe"

typedef QList<QStringList*>  projectArray_t;


class MSBuildThread : public QThread
{
    Q_OBJECT
public:
    void SetProgram(QString &p);
    void SetProject(QString &p);
    void SetTarget(QString &t);
    void SetLevel(int l);

    virtual void run();

    QByteArray readOutPut();
    QString readErrMsg();

signals:
    void ready2ReadOutput(MSBuildThread *thread);
    void almostFinished(int level, QString path, int exitcode);
    void taskProcessStatus(MSBuildThread *thread);

private:
    QString     path;
    QString     project;
    QString     target;
    int         level;
    QProcess    *msbuild;

    QString     errMsg;

private slots:
    void hanldTaskReadOutPutSingle();
};


class DisplayMSBuild;
class MSBuildTask:public QObject {
    Q_OBJECT
public:
    MSBuildTask();
    virtual ~MSBuildTask();

    bool setDisplayDialog(int projNum = 0);
    bool setTask(int s, projectArray_t *projSortByLevel);
    bool startTask();
    void cancelTask();
    static void cleanProjectArray(projectArray_t *array);

private:
    enum {
        none = 0,
        started,
        exiting
    } taskStatus;

    DisplayMSBuild *display;
    int size;
    projectArray_t *projectListArray;
    DisplayMSBuild *progress;

signals:
    void taskComplete(bool status);

private slots:
    void callMSBuild(int level);
    void msbuildThreadFinished(int level, QString path, int exitCode);
};

#endif // MSBUILDTHREAD_H
