#include "../include/praserCSprojFile.h"

csProjPrase::csProjPrase(QString &fileName)
{
    projPath = fileName;

    rootElement = NULL;

    load = fx.LoadFile(fileName.toLatin1().data());
	if (isLoaded()) {
		rootElement = fx.FirstChildElement("Project");
    }

    file = fileName.mid(fileName.lastIndexOf('/') + 1, fileName.lastIndexOf('.') - fileName.lastIndexOf('/') -1);
}


int csProjPrase::Do()
{
	if (!rootElement)
		return -1;

    if (file.contains(QString("test"), Qt::CaseInsensitive))
        return -2;

    //getOutputName();
    getProjectName();
    if (projName.contains(QString("test"), Qt::CaseInsensitive)) {
        return -2;
    }
	getProjectTypeAndGUID();
    praserProjRef();
	getDependency();
    getProjPostFix();
	return 0;
}

void csProjPrase::getProjectTypeAndGUID()
{
	TiXmlElement* tmp = NULL;
	for (tmp = rootElement->FirstChildElement("PropertyGroup");
		tmp;
		tmp = tmp->NextSiblingElement("PropertyGroup")) {
			TiXmlElement* type =tmp->FirstChildElement("OutputType");
			if (type &&type->FirstChild()) {
                projType = QString(type->FirstChild()->Value());
			}

			TiXmlElement* guid =tmp->FirstChildElement("ProjectGuid");
			if (guid &&guid->FirstChild()) {
                projGuid = QString(guid->FirstChild()->Value());
			}

		if (projType.length() >= 0 && projGuid.length() >= 0) {
            return;
		}
	}

}

void csProjPrase::getProjectName()
{
    TiXmlElement* tmp = NULL;
    for (tmp = rootElement->FirstChildElement("PropertyGroup");
        tmp;
        tmp = tmp->NextSiblingElement("PropertyGroup")) {
            TiXmlElement* name =tmp->FirstChildElement("AssemblyName");
            if (name && name->FirstChild()) {
                projName = QString(name->FirstChild()->Value());
                return;
            }
    }

}


void csProjPrase::getDependency()
{
	TiXmlElement* tmp = NULL;

	for (tmp = rootElement->FirstChildElement("ItemGroup");
		tmp;
		tmp = tmp->NextSiblingElement("ItemGroup")) {

        //Ref dll
        for (TiXmlElement* projRefList =tmp->FirstChildElement("Reference");
            projRefList;
            projRefList = projRefList->NextSiblingElement("Reference")) {
            TiXmlElement* path = projRefList->FirstChildElement("HintPath");
            if (path &&path->FirstChild()) {
                QString tmp(path->FirstChild()->Value());
                int rpoint = tmp.lastIndexOf('.');
                int rsh = tmp.lastIndexOf('\\');

                if (rpoint <= rsh)  {
                    qDebug() << "csproj Unknown referenc type, ref:" << tmp << endl;
                } else {
                    rsh = rsh < 0 ? 0 : rsh;
                    tmp = tmp.mid(rsh + 1, tmp.lastIndexOf('.') - rsh -1);

                    if (projDepend.contains(tmp) == false) {
                        projDepend.push_back(tmp);
                    }
                }
            }
        }

        //Ref proj
        for (TiXmlElement* projRefProj =tmp->FirstChildElement("ProjectReference");
            projRefProj;
            projRefProj = projRefProj->NextSiblingElement("ProjectReference")) {
            QString projName(projRefProj->Attribute("Include"));
            projName = projName.mid(projName.lastIndexOf('\\')+1, projName.lastIndexOf('.') - projName.lastIndexOf('\\') -1);
            if (projDepend.contains(projName) == false) {
                projDepend.push_back(projName);
            }
        }
    }
}
