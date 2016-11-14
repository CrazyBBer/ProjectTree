#include "../include/praserVCprojFile.h"

vcProjPrase::vcProjPrase(QString &fileName)
{
    projPath = fileName;
    load = fx.LoadFile(fileName.toLatin1().data());

    rootElement = NULL;

	if (isLoaded()) {
		rootElement = fx.FirstChildElement("Project");
	}

    file = fileName.mid(fileName.lastIndexOf('/') + 1, fileName.lastIndexOf('.') - fileName.lastIndexOf('/') -1);
}

int vcProjPrase::Do()
{
	if (!rootElement)
		return -1;

    if (file.contains(QString("test"), Qt::CaseInsensitive))
        return -2;

    getOutputName();
    if (projName.contains(QString("test"), Qt::CaseInsensitive)) {
        return -2;
    }
	getProjectGuid();
	getProjectType();
    getDependency();
	praserProjRef();

    getProjPostFix();
	
	return 0;
}

void vcProjPrase::getProjectGuid()
{
	TiXmlElement* tmp = NULL;
	for (tmp = rootElement->FirstChildElement("PropertyGroup");
		tmp;
		tmp = tmp->NextSiblingElement("PropertyGroup")) {
			if (!tmp->Attribute("Label")) {
				continue;
			}
			if (strcmp(tmp->Attribute("Label"), VCPPGLOUBLELAB)) {
				continue;
			}

			TiXmlElement* guid =tmp->FirstChildElement("ProjectGuid");
			if (guid && guid->FirstChild()) {
                projGuid = QString(guid->FirstChild()->Value());
				return;
			}
	}

}

void vcProjPrase::getOutputName()
{
    TiXmlElement* tmp = NULL;
    for (tmp = rootElement->FirstChildElement("ItemDefinitionGroup");
        tmp;
        tmp = tmp->NextSiblingElement("ItemDefinitionGroup")) {
            if (!tmp->Attribute("Condition")) {
                continue;
            }
            if (strcmp(tmp->Attribute("Condition"), VCPPCONDITION)) {
                continue;
            }

            TiXmlElement* link =tmp->FirstChildElement("Link");
            if (link) {
                TiXmlElement *Output = link->FirstChildElement("OutputFile");
                if (Output && Output->FirstChild()) {
                    QString out = QString(Output->FirstChild()->Value());
                    if (out.contains('.')) {
                        int start1 = out.lastIndexOf('\\');
                        int start2 = out.lastIndexOf('/');
                        if (start1 < start2)
                            start1 = start2;
                        if (start1 < 0)
                            start1 = -1;
                        projName = out.mid(start1 + 1, out.lastIndexOf('.') - start1-1);
                    } else {
                        int start1 = projPath.lastIndexOf('\\');
                        int start2 = projPath.lastIndexOf('/');
                        if (start1 < start2)
                            start1 = start2;
                        if (start1 < 0)
                            start1 = -1;
                        projName = projPath.mid(start1 + 1, projPath.lastIndexOf('.') - start1-1);
                    }
                    return;
                }
            }
    }

    if (projName.isEmpty())
        projName = file.left(file.indexOf('.'));
}

void vcProjPrase::getProjectType()
{
	TiXmlElement* tmp = NULL;
    for (tmp = rootElement->FirstChildElement("PropertyGroup");
		tmp;
        tmp = tmp->NextSiblingElement("PropertyGroup")) {
			if (!tmp->Attribute("Condition")) {
				continue;
			}
			if (strcmp(tmp->Attribute("Condition"), VCPPCONDITION)) {
				continue;
			}

			TiXmlElement* type =tmp->FirstChildElement("ConfigurationType");
			if (type && type->FirstChild()) {
                projType = QString(type->FirstChild()->Value());
				return;
			}
	}

}

void vcProjPrase::getDependency()
{
	TiXmlElement* tmp = NULL;

	for (tmp = rootElement->FirstChildElement("ItemDefinitionGroup");
		tmp;
		tmp = tmp->NextSiblingElement("ItemDefinitionGroup")) {
			if (!tmp->Attribute("Condition")) {
				continue;
			}
			if (strcmp(tmp->Attribute("Condition"), VCPPCONDITION)) {
				continue;
			}

			TiXmlElement* link =tmp->FirstChildElement("Link");
			if (link) {
				TiXmlElement* dep = link->FirstChildElement("AdditionalDependencies");
				if (dep && dep->FirstChild()) {
                    QString tmp(dep->FirstChild()->Value());
                    for (;tmp.isEmpty() == false;tmp = tmp.right(tmp.length() - tmp.indexOf(';') - 1)) {
                        QString dl;
                        if (tmp.contains(';'))
                            dl = tmp.left(tmp.indexOf(';'));
                        else
                            dl = tmp;

                        int start1 = dl.lastIndexOf('\\');
                        int start2 = dl.lastIndexOf('/');
                        if (start1 < start2)
                            start1 = start2;

                        if (dl.contains('.')) {
                            dl = dl.mid(start1+1, dl.lastIndexOf('.') -start1-1 );

                            if (projDepend.contains(dl) == false)
                                projDepend.push_back(dl);
                        }
                        if (tmp.contains(';')==false)
                            break;
					}
					return;
				}
			}
	}

}
