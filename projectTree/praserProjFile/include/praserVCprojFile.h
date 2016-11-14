#pragma once
#include "praser.h"

#define VCPPGLOUBLELAB	"Globals"
#define VCPPCONDITION	"'$(Configuration)|$(Platform)'=='Release|Win32'"


class vcProjPrase: public praser {
public:
    vcProjPrase(QString &fileName);
	int Do();

    void getProjPostFix(){
        if (!projPostFix.isEmpty()) {
            return;
        }

		if (projType == QString("StaticLibrary")) {
			projPostFix = QString(".lib");
		}else if(projType == QString("DynamicLibrary")){
			projPostFix = QString(".dll");
		}else if(projType == QString("Application")){
			projPostFix = QString(".exe");
		}else {
            qDebug() << "Unknown vc projType:" << projType << endl;
			projPostFix = QString("UNKNOWN");
		}
        return;
	}

	const QString& getProjGuid() const {
		return projGuid;
	}

private:
	void getProjectGuid();
	void getProjectType();
	void getDependency();
    void getOutputName();
};
