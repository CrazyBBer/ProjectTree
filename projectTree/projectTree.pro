#-------------------------------------------------
#
# Project created by QtCreator 2015-11-24T19:15:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = projectTree
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mscene.cpp \
    praserProjFile/source/praserCSprojFile.cpp \
    praserProjFile/source/praserSLNFile.cpp \
    praserProjFile/source/praserVCprojFile.cpp \
    tinyxml/tinystr.cpp \
    tinyxml/tinyxml.cpp \
    tinyxml/tinyxmlerror.cpp \
    tinyxml/tinyxmlparser.cpp \
    mlineedit.cpp \
    mcfgdlg.cpp \
    mtablewidget.cpp \
    msbuildthread.cpp \
    displaymsbuild.cpp

HEADERS  += mainwindow.h \
    mscene.h \
    mTextItem.h \
    praserProjFile/include/config.h \
    praserProjFile/include/praser.h \
    praserProjFile/include/praserCSprojFile.h \
    praserProjFile/include/praserSLNFile.h \
    praserProjFile/include/praserVCprojFile.h \
    tinyxml/tinyxml.h \
    mlineedit.h \
    mcfgdlg.h \
    projitem.h \
    mtablewidget.h \
    msbuildthread.h \
    displaymsbuild.h

FORMS    += mainwindow.ui \
    configdlg.ui

INCLUDEPATH += -I./ -I./tinyxml -I./praserProjFile/include
