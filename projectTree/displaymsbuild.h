#ifndef DISPLAYMSBUILD_H
#define DISPLAYMSBUILD_H

#include <QDialog>
#include <QTextBrowser>
#include <QProgressBar>


class MSBuildThread;

class DisplayMSBuild : public QDialog
{
public:
    DisplayMSBuild(int total, bool procMode = false);
    virtual ~DisplayMSBuild();

    void setTotalNum(int num);
    void viewSumLog();
    void clean();

private:
    enum {
        showProgress = 0,
        showSum,
        showErr,
    } taskStatus;
    QTextBrowser    browser;
    QProgressBar    *bar;

    int projectNum;
    int doneNum;

public slots:
    void handleMSBuildMsg(MSBuildThread *msbuild);
    void handldMSBuildFinish(int level, QString path, int exitCode);
    void handleMSBuildErr(MSBuildThread *msbuild);
};

#endif // DISPLAYMSBUILD_H
