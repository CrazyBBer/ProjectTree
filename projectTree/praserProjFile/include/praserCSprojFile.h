#pragma once
#include "praser.h"
#include <QDebug>

#define CSPROJCONDITION	" '$(Configuration)|$(Platform)' == 'Release|AnyCPU' "

class csProjPrase: public praser {
public:
        csProjPrase(QString &fileName);
	int Do();

    void getProjPostFix(){
        if (!projPostFix.isEmpty()) {
            return;
        }

		if (projType == QString("Library") || projType == QString("library")) {
			projPostFix =  QString(".dll");
		}else if(projType == QString("Exe") || projType == QString("WinExe")){
			projPostFix =  QString(".exe");
		}else {
            qDebug() << "Unknown cs projType:" << projType<< endl;
			projPostFix = QString("UNKNOWN");
        }
        return;
	}

	
private:
	void getProjectTypeAndGUID();
	void getDependency();
    void getProjectName();
};
