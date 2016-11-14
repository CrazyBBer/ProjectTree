#include "praserProjFile/include/praserSLNFile.h"
#include <direct.h>


slnPrase::slnPrase(const QString &fileName):slnfile(fileName)
{
    fs.open(fileName.toLatin1().data(), std::ios_base::in);
    int lastPos = slnfile.lastIndexOf('/');
    if (lastPos < slnfile.lastIndexOf('\\'))
        lastPos = slnfile.lastIndexOf('\\');
    slnPath = slnfile.left(lastPos +1 );
    slnName = slnfile.mid(lastPos + 1, slnfile.lastIndexOf('.') - slnfile.lastIndexOf('\\')-1);
}

int slnPrase::Do()
{
    if (loadSLNFile() == false) {
        qDebug() << "Open sln file failed: ";
		return -1;
	}

    if (loadProjFile() == false) {
        qDebug() << "loadProjFile: failed ";
		return -2;
    }

    if (SortProjList() == false) {
        qDebug() << "SortProjList:  failed";
		return -3;
    }

	return 0;
}

bool slnPrase::readProjInfo(char *headInfo)
{
	projInfo_t *inf = new projInfo_t();
	if (!inf) {
        qDebug() << "Create ProjInfo_t Failed!!!!!" << endl;
        return false;
	}

    if (!getProjInfo(QString(headInfo), inf)) {
        delete inf;
        return false;
	}


	if (!plistFindbyGuid(slnProjList, inf->projGuid))
		slnProjList.push_back(inf);
	else
        qDebug() << "projAlready in list!!!  " << inf->projGuid  << endl;

	char buf[1024];

	while (!fs.eof()) {
		memset(buf, 0, 1024);
		fs.getline(buf, 1024);
		if (!strlen(buf))
			continue;
		
		if (strncmp(buf, STRUCTEND, strlen(STRUCTEND)) == 0)
            return true;

        QString dep(buf);

        if (dep.contains('{')) {
            QString guid = dep.mid(dep.indexOf('{'), dep.indexOf('}') - dep.indexOf('{')+1);
			inf->projReleate.push_back(guid);
		}
	}

    return true;
}


bool slnPrase::getProjInfo(QString buf, projInfo_t *inf)
{
    QString typeguid = buf.mid(buf.indexOf('{'), buf.indexOf('}') - buf.indexOf('{') + 1);
    if (typeguid == QString(VCPROJGUID))
		inf->projType = CPLusPlusProj;
    else if(typeguid ==QString(CSPROJGUID))
		inf->projType = CShelperProj;
    else if(typeguid ==QString(WEBSITEGUID)) {
		inf->projType = WebSiteProj;	// not support
        return false;
	}
    else if(typeguid ==QString(SETUPPROJGUID)) {
		inf->projType = SetUpProj;	// not and no need support
        return false;
	}
    else if (typeguid ==QString(SOLUTIONDIRGUID)) {
        inf->projType = SolutionFolder;
        return false;
	}
	else {
        qDebug() << "UNknown ProjType, Please concern this! typeguid:" << typeguid << endl;
		inf->projType = OtherType;
        return false;
	}

    inf->projName = buf.mid(buf.indexOf('=') +1, buf.indexOf(',') - buf.indexOf('=')-1);
    inf->projName = inf->projName.mid(inf->projName.indexOf('\"')+1, inf->projName.lastIndexOf('\"')-inf->projName.indexOf('\"')-1);
    inf->projPath = buf.mid(buf.indexOf(',')+1, buf.lastIndexOf(',') - buf.indexOf(',')-1);
    inf->projPath = inf->projPath.mid(inf->projPath.indexOf('\"')+1, inf->projPath.lastIndexOf('\"')-inf->projPath.indexOf('\"')-1);
    inf->projGuid = buf.mid(buf.lastIndexOf('{'), buf.lastIndexOf('}') - buf.lastIndexOf('{') +1);

    return true;
}

bool slnPrase::loadSLNFile(QStringList &projList)
{
    if (!fs.is_open()) {
        return false;
    }

    char buf[1024];

    while (!fs.eof()) {
        memset(buf, 0, 1024);
        fs.getline(buf, 1024);
        if (!strlen(buf))
            continue;
        if (strncmp(buf, PROJINFOHEAD, strlen(PROJINFOHEAD)) == 0) {
            QString proj(buf);
            QString projGuid = proj.mid(proj.indexOf('{'), proj.indexOf('}') - proj.indexOf('{')+1);
            bool continueFlag = true;
            if (projGuid == "{2150E333-8FDC-42A3-9474-1A3956D46DE8}") {
                ;
            }else if (projGuid == "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}" ||
                      projGuid == "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}") {
                continueFlag = false;
            } else {
                qDebug() << "Unknown Project Item Guid: " << projGuid;
            }

            if (continueFlag)
                continue;

            proj = proj.mid(proj.indexOf(',')+1, proj.lastIndexOf(',') - proj.indexOf(',')-1);
            proj = proj.mid(proj.indexOf('"')+1, proj.lastIndexOf('"') - proj.indexOf('"')-1);
            proj = slnPath + proj;
            while (proj.indexOf('\\')>=0) {
                proj.replace(proj.indexOf('\\'), 1, QString('/'));
            }

            if (projList.contains(proj) == false)
                projList.push_back(proj);
        }
    }

    return true;
}

bool slnPrase::loadSLNFile()
{
	if (!fs.is_open()) {
        return false;
	}

	char buf[1024];

	while (!fs.eof()) {
		memset(buf, 0, 1024);
		fs.getline(buf, 1024);
		if (!strlen(buf))
			continue;
		if (strncmp(buf, PROJINFOHEAD, strlen(PROJINFOHEAD)) == 0)
			readProjInfo(buf);
	}

    return true;
}

bool slnPrase::loadProjFile()
{
    QString projPath;
	for (pit it = slnProjList.begin(); it != slnProjList.end(); it++){
        if ((*it)->projPath.contains(':') == false)
			projPath = slnPath +"\\"+ (*it)->projPath;
		else
			projPath = (*it)->projPath;

		if ((*it)->projType == CPLusPlusProj) {
            vcProjPrase proj(projPath);
			if (!projListFindbyGUID(ProjList, proj)) {
				proj.Do();
				ProjList.push_back(proj);
				proj.exportRef2SLNprojInfo(slnProjList);
                (*it)->projPostFix = proj.projPostFix;
			}
		}
		else if ((*it)->projType == CShelperProj) {
            csProjPrase proj(projPath);
			if (!projListFindbyGUID(ProjList, proj)) {
				proj.Do();
				ProjList.push_back(proj);
				proj.exportRef2SLNprojInfo(slnProjList);
                (*it)->projPostFix = proj.projPostFix;
			}
		}
		else if ((*it)->projType == WebSiteProj) {
            qDebug() << "TODO: add WebSiteProj praser function" << endl;
		}
		else {
            qDebug() << "Unknown projType! GUID:"<< (*it)->projGuid << endl;
		}

	}
    return true;
}

bool slnPrase::SortProjList()
{
	pit it = slnProjList.begin();

	while (it != slnProjList.end()) {
		if ((*it)->projReleate.size() == 0) {
			it++;
			continue;
		}

		pit newPos = it;
		for (sit si = (*it)->projReleate.begin(); si != (*it)->projReleate.end(); si++) {
			pit pos = plistFind(newPos, slnProjList.end(), *si);
			if (pos !=  slnProjList.end()) {
				newPos = pos;
			}
		}
        if (newPos != it) {
            slnProjList.swap(slnProjList.indexOf(*newPos), slnProjList.indexOf(*it));
		}else
			it++;
	}

    return true;
}

bool slnPrase::refListCheckbyName(slist &refList, QString name)
{
	for (sit si = refList.begin(); si != refList.end(); si++) {
		pit pi = plistFind(slnProjList.begin(), slnProjList.end(), *si);
		if (pi != slnProjList.end()) {
            if ((*pi)->projName == name.mid(0, name.lastIndexOf('.'))) {
                return true;
			}
		}
	}
    return false;
}
bool slnPrase::recurSionCheckRepeat(slist &projRefList, slist &projDepList, QString &proj)
{
	for (sit si = projDepList.begin(); si != projDepList.end(); si++) {
		praser p;
		if (projListFindbyNAME(slnProjList, ProjList, *si, p)) {
			if (caseCmp(p.getProjDependence(), proj)) {
                return true;
			} else if (refListCheckbyName(p.getProjRef(), proj)) {
                return true;
			} else if (recurSionCheckRepeat(p.getProjRef(), p.getProjDependence(), proj)) {
                return true;
			}
		}
	}

    //check projReference list one by one, untile it's true
	for (sit si = projRefList.begin(); si != projRefList.end(); si++) {
		// ref is proj.guid list, find praser obj first
		praser p;
		if (projListFindbyGUID(ProjList, *si, p)){
			if (caseCmp(p.getProjDependence(), proj)) {
                return true;
			} else if (refListCheckbyName(p.getProjRef(), proj)) {
                return true;
			} else if (recurSionCheckRepeat(p.getProjRef(), p.getProjDependence(), proj)) {
                return true;
			}
		}
	}

    return false;
}


void slnPrase::addDependlist(praser &p, slist &deplist)
{
	slist dep = p.getProjDependence();
	for (sit si = dep.begin(); si != dep.end(); si++) {
        if (deplist.contains(*si) == false){	// none repeate
            if (recurSionCheckRepeat(p.getProjRef(), p.getProjDependence(), *si) == false) {
				deplist.push_back(*si);
			}
		}
	}

	slist projRefList = p.getProjRef();
	for (sit si = projRefList.begin(); si != projRefList.end(); si++) {
		pit pi = plistFind(slnProjList.begin(), slnProjList.end(), *si);
		if (pi != slnProjList.end()) {
            QString wholeName = (*pi)->projName + (*pi)->projPostFix;
            if (deplist.contains(wholeName) == false){	// none repeate
                if (recurSionCheckRepeat(p.getProjRef(), p.getProjDependence(), wholeName) == false) {
					deplist.push_back(wholeName);
				}
			}
		}
	}
}

bool slnPrase::exportProjInfo(slist &l, xmap &m, slist &serviceList)
{
	// export Project one by one accroding slnProjList that after sorted
    for (pit it = slnProjList.begin(); it != slnProjList.end(); it++) {
        if (l.contains((*it)->projGuid))
			continue;	//already in global projList

		praser p;
        if (projListFindbyGUID(ProjList, (*it)->projGuid, p) == false) {
            qDebug() << "Warn: connot found projDetail from praser list :"<< (*it)->projGuid << endl;
			continue;	//error condition
        }
		slist deplist;
		addDependlist(p, deplist);

        //export to GLOBAL
        if (l.contains((*it)->projName+(*it)->projPostFix)){
            qDebug() << "Warnning: sampe ProjName in gProjList:" << (*it)->projName << (*it)->projPostFix << endl;
		}
        xmit xi = m.find((*it)->projName);
        if (xi != m.end()) {
			;//Error Condition
        } else {
            m.insert((*it)->projName+(*it)->projPostFix, deplist);
            l.push_back((*it)->projName+(*it)->projPostFix);
            if ((*it)->projPostFix == ".exe" && nStrCmp((*it)->projName.toLatin1().data(), "Test")!=0) {
                serviceList.push_back((*it)->projName+(*it)->projPostFix);
			}
		}
	}
    return true;
}

//bool slnPrase::dumpSLNReleateInfo()
//{
//    QString fileName = PROJRELEATE + QString("\\") + slnName+".Relation.xml";
//	TiXmlDocument doc;

//	TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "", "");
//	TiXmlElement *slnElement = new TiXmlElement("Solution");

//	for (pit it = slnProjList.begin(); it != slnProjList.end(); it++) {
//		TiXmlElement *projEle = new TiXmlElement("Project");
//        projEle->SetAttribute("ProjName", (*it)->projName.toLatin1().data());
//		// ProjReleate
//		if (!(*it)->projReleate.empty()) {
//			TiXmlElement *Ref = new TiXmlElement("ProjectReference");
//			for (sit si = (*it)->projReleate.begin(); si != (*it)->projReleate.end(); si++) {
//				pit pji = plistFind(slnProjList.begin(), slnProjList.end(), *si);
//				if (pji != slnProjList.end()) {
//					TiXmlElement *proj = new TiXmlElement("Project");
//                    TiXmlText *text = new TiXmlText((*pji)->projName.toLatin1().data());
//					proj->LinkEndChild(text);
//					Ref->LinkEndChild(proj);
//				}else{
//                    qDebug() << "warn: Project Releate not in this Solution!!!" << endl;
//				}
//			}
//			projEle->LinkEndChild(Ref);
//		}
//		// DLL Releate
//		slist projdep;
//		praser p;
//		if ((*it)->projType == CPLusPlusProj || (*it)->projType == CShelperProj) {
//			if (projListFindbyGUID(ProjList, (*it)->projGuid, p)){
//				projdep = p.getProjDependence();
//			}
//		}

//		if (!projdep.empty()) {
//			TiXmlElement *depEle = new TiXmlElement("Dependence");
//			for (sit si = projdep.begin(); si != projdep.end(); si++) {
//				TiXmlElement *lib = new TiXmlElement("library");
//                TiXmlText *text = new TiXmlText((*si).toLatin1().data());
//				lib->LinkEndChild(text);
//				depEle->LinkEndChild(lib);
//			}
//			projEle->LinkEndChild(depEle);
//		}

//		slnElement->LinkEndChild(projEle);
//	}

//	doc.LinkEndChild(dec);
//	doc.LinkEndChild(slnElement);
//    doc.SaveFile(fileName.toLatin1().data());

//    return true;
//}
