#pragma once
#include <fstream>
#include "praser.h"
#include "praserCSprojFile.h"
#include "praserVCprojFile.h"


#define PROJRELEATE		"ProjReleate"

#define PROJINFOHEAD	"Project("
#define PROJSECTION		"ProjectSection"
#define STRUCTEND		"End"


class slnPrase{
public:
    slnPrase(const QString &fileName);
    bool loadSLNFile(QStringList &projList);
	int Do();
    //bool dumpSLNReleateInfo();
    bool exportProjInfo(slist &l, xmap &m, slist &serviceList);

private:
    bool getProjInfo(QString buf, projInfo_t *inf);
    bool readProjInfo(char *headInfo);
    bool loadSLNFile();
    bool loadProjFile();
    bool SortProjList();
    bool recurSionCheckRepeat(slist &projRefList, slist &dep, QString &proj);
	void addDependlist(praser &p, slist &deplist);
    bool refListCheckbyName(slist &refList, QString name);

	projInfoList	slnProjList;
	projList		ProjList;
    std::fstream fs;
    QString slnfile;
    QString slnPath;
    QString slnName;
};
