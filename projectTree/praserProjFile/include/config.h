#pragma once

#include <stdio.h>
#include <QString>
#include <QList>
#include <algorithm>
#include <QDebug>

#define FILENUMMAX	300
#define CFGFILENAME "slnList.cfg"

FILE inline *createConfig(char *cfgFileName)
{
	FILE *fp = NULL;
    fopen_s(&fp, cfgFileName, "w+");
	if (fp == NULL) {
		return NULL;
	}

	fputs("#PATH, example:C:\\HoneyWell, max path len=255\n", fp);
	fputs("PATH:\n", fp);
	fputs("#SLN, example:\\xxx.sln or C:\\HoneyWell\\xxx.sln, max path len=255\n", fp);
	fputs("SLN:\n", fp);
	fseek(fp, 0, SEEK_SET);
	return fp;
}


int inline readConfig(QList<QString> &slnList)
{
	FILE *fpconfig = NULL;
	fopen_s(&fpconfig, CFGFILENAME, "r");
	if (fpconfig == NULL) {
		fpconfig = createConfig(CFGFILENAME);
	}

	if (!fpconfig)
		return -1;

	fseek(fpconfig, 0, SEEK_END);
	int filesize = ftell(fpconfig);
	fseek(fpconfig, 0, SEEK_SET);

    QString Path;
    char buf[_MAX_PATH] = { 0 };

	do {
		fgets(buf, _MAX_PATH, fpconfig);
		if (strlen(buf) <= 0)
			continue;

		if (buf[strlen(buf)-1] == '\n')
            buf[strlen(buf)-1] = '\0';
        if (buf[0] == '#') {
			continue;
        }
		else if (strncmp(buf, "PATH:", strlen("PATH:")) == 0) {
            Path = QString(&buf[strlen("PATH:")]);
            if (Path.contains(":") == false) {
                qDebug() << "Please fill \"PATH:\" by right format! " << Path << endl;
				goto ERR;
			}
            if (Path.lastIndexOf('\\') != Path.length() - 1) {
                Path += '\\';
			}
		}else if (strncmp(buf, "SLN:", strlen("SLN:")) == 0) {
            QString sln(&buf[strlen("SLN:")]);
            if (sln.contains('.') == false || sln.rightRef(sln.lastIndexOf('.'))==".sln") {
                qDebug() << "Please fill \"SLN:\" in right format! " << sln << endl;
				goto ERR;
			}
            if (sln.contains(':') == false) {
                sln = Path + sln;
            }
            if (slnList.contains(sln) == false)
				slnList.push_back(sln);
		}
	} while (ftell(fpconfig) < filesize);

	fclose(fpconfig);
	fpconfig = NULL;

	return 0;
ERR:
	fclose(fpconfig);
	fpconfig = NULL;
	return -1;
}



int inline readConfig(QList<QString> &slnList, QString &cfgFileName)
{
    FILE *fpconfig = NULL;
    fopen_s(&fpconfig, cfgFileName.toLatin1().data(), "r");
    if (fpconfig == NULL) {
        return -1;
    }

    fseek(fpconfig, 0, SEEK_END);
    int filesize = ftell(fpconfig);
    fseek(fpconfig, 0, SEEK_SET);

    QString Path;
    char buf[_MAX_PATH] = { 0 };

    do {
        fgets(buf, _MAX_PATH, fpconfig);
        if (strlen(buf) <= 0)
            continue;

        if (buf[strlen(buf)-1] == '\n')
            buf[strlen(buf)-1] = '\0';
        if (buf[0] == '#') {
            continue;
        }
        else if (strncmp(buf, "PATH:", strlen("PATH:")) == 0) {
            Path = QString(&buf[strlen("PATH:")]);
            if (Path.contains(":") == false) {
                qDebug() << "Please fill \"PATH:\" by right format! " << Path << endl;
                goto ERR;
            }
            if (Path.lastIndexOf('\\') != Path.length() - 1) {
                Path += '\\';
            }
        }else if (strncmp(buf, "SLN:", strlen("SLN:")) == 0) {
            QString sln(&buf[strlen("SLN:")]);
            if (sln.contains('.') == false || sln.rightRef(sln.lastIndexOf('.'))==".sln") {
                qDebug() << "Please fill \"SLN:\" in right format! " << sln << endl;
                goto ERR;
            }
            if (sln.contains(':') == false) {
                sln = Path + sln;
            }
            if (slnList.contains(sln) == false)
                slnList.push_back(sln);
        }
    } while (ftell(fpconfig) < filesize);

    fclose(fpconfig);
    fpconfig = NULL;

    return 0;
ERR:
    fclose(fpconfig);
    fpconfig = NULL;
    return -1;
}

