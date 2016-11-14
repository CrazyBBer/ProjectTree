#pragma  once


#include <iostream>
#include <QList>
#include <QMap>
#include <QString>
#include <algorithm>
#include "tinyxml/tinyxml.h"
#include <QDebug>


#define SOLUTIONDIRGUID	"{2150E333-8FDC-42A3-9474-1A3956D46DE8}"
#define CSPROJGUID		"{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
#define VCPROJGUID		"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
#define WEBSITEGUID		"{E24C65DC-7377-472B-9ABA-BC803B73C61A}"
#define SETUPPROJGUID	"{54435603-DBB4-11D2-8724-00A0C9A8B90C}"


typedef enum {
	CPLusPlusProj,
	CShelperProj,
	SolutionFolder,
	WebSiteProj,
	SetUpProj,
	OtherType
}projType_e;

typedef struct {
	projType_e	projType;
    QString	projGuid;
    QString	projName;
    QString	projPath;
    QString	projPostFix;
    QList<QString>	projReleate;		// relation in sln file
} projInfo_t;

typedef QList<projInfo_t*> projInfoList;
typedef QList<projInfo_t*>::iterator pit;
typedef QList<QString> slist;
typedef QList<QString>::iterator sit;
typedef QMap<QString, slist> xmap;
typedef QMap<QString, slist>::iterator xmit;

pit inline plistFind(pit bi, pit ei, QString guid);

class praser {
public:
    virtual bool isLoaded(){return load == true;}
    virtual int Do(){return 0;}

	void praserProjRef()
	{
		TiXmlElement* itemGroup = NULL;
		TiXmlElement* projRefelem = NULL;

		for (itemGroup = rootElement->FirstChildElement("ItemGroup");
			itemGroup;
			itemGroup = itemGroup->NextSiblingElement("ItemGroup")) {
				for (projRefelem = itemGroup->FirstChildElement("ProjectReference");
					projRefelem;
					projRefelem = projRefelem->NextSiblingElement("ProjectReference")) {
						TiXmlNode* guid = projRefelem->FirstChild("Project");
                        if (guid &&guid->FirstChild()) {
                            QString g = guid->FirstChild()->Value();
                            if (projRef.contains(g.toUpper()) == false)
                                projRef.push_back(g.toUpper());
						}
				}
		}
	}

	void exportRef2SLNprojInfo(projInfoList &slnProjList)
	{
		pit it = plistFind(slnProjList.begin(), slnProjList.end(), projGuid);
		if (it == slnProjList.end()) {
			//g_Result.warn++;
            qDebug() << "warn: project not in sln list! GUID:" << projGuid << endl;
			return;
		}

		for (sit itb = projRef.begin();itb != projRef.end(); itb++) {
			sit si = (*it)->projReleate.begin();
			for (; si != (*it)->projReleate.end(); si++) {
				if (*si == *itb)
					break;
			}
			if (si == (*it)->projReleate.end())
				(*it)->projReleate.push_back(*itb);
		}
		return;
	}

	slist &getProjDependence() {
		return projDepend;
	}
	slist &getProjRef() {
		return projRef;
	}

    const QString& getProjGuid() const {
		return projGuid;
	}


    QString projType;
    QString projGuid;
    QString projPostFix;
    QString projName;
    QString projPath;
    slist	projDepend;		// dependence dll list
    slist	projRef;		// reference proj list


    QString file;

protected:
	TiXmlDocument fx;
    bool	load;
    TiXmlElement *rootElement;
};


typedef QList<praser> projList;
typedef QList<praser>::iterator pjIt;


int inline nStrCmp(const char *src, const char *sub)
{
	if (!src || !sub || !strlen(src) || !strlen(sub))
		return -1;

	size_t subLen = strlen(sub);

	while (strlen(src) >= subLen) {
		size_t cnt = 0;
		while (cnt < subLen) {
			if (tolower(src[cnt]) != tolower(sub[cnt]))
				break;
			cnt++;
		}
		if (cnt == subLen)
			return 0;

		if (cnt)
			src+=cnt;
		else
			src++;
	}

	return  1;
}

bool inline caseCmp(slist &l, QString wholeLibName)
{
    wholeLibName = wholeLibName.left(wholeLibName.lastIndexOf('.'));

	for (sit si = l.begin(); si != l.end(); si++) {
        QString str = *si;
        str = str.left(str.lastIndexOf('.'));
		if (str == wholeLibName)
            return true;
	}

    return false;
}


bool inline plistFindbyGuid(projInfoList &l, QString &guid)
{
	pit it = l.begin();
	pit ite = l.end();

	for (; it != ite; it++) {
		if ((*it)->projGuid == guid)
            return true;
	}

    return false;
}

pit inline plistFind(pit bi, pit ei, QString guid)
{
	for (; bi != ei; bi++) {
		if ((*bi)->projGuid == guid)
			break;
	}

	return bi;
}

pit inline plistFindbyName(pit bi, pit ei, QString name)
{
	for (; bi != ei; bi++) {
        if ((*bi)->projName == name.left(name.lastIndexOf('.')))
			break;
	}

	return bi;
}


sit inline slistFind(slist &l, QString str)
{
	sit bi = l.begin();
	sit ei = l.end();

	for (; bi != ei; bi++) {
		if (*bi == str)
			break;
	}

	return bi;
}


bool inline projListFindbyGUID(projList &l, praser &proj)
{
	pjIt it = l.begin();
	pjIt ite = l.end();

	for (; it != ite; it++) {
		if ((*it).getProjGuid() == proj.getProjGuid())
            return true;
	}

    return false;
}


bool inline projListFindbyGUID(projList &l, QString guid, praser &proj)
{
	pjIt it = l.begin();
	pjIt ite = l.end();

	for (; it != ite; it++) {
		if ((*it).getProjGuid() ==guid) {
			proj = *it;
            return true;
		}
	}

    return false;
}

bool inline projListFindbyNAME(projInfoList &pInfol, projList &pl, QString name, praser &proj)
{
	pit pi = plistFindbyName(pInfol.begin(),pInfol.end(), name);
	if (pi == pInfol.end())
        return false;

	if (projListFindbyGUID(pl, (*pi)->projGuid, proj)) {
        return true;
	}

    return false;
}
