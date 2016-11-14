#include "mcfgdlg.h"
#include "ui_configdlg.h"
#include "tinyxml/tinyxml.h"
#include <QFileDialog>
#include <QDebug>

mCfgDlg::mCfgDlg(QWidget *parent):QDialog(parent),ui(new Ui::Dialog)
{
    ui->setupUi(this);

    btnProc = new QPushButton(this);
    btnProc->setGeometry(303, 301,75,25);
    btnProc->setText(tr("解析文件"));

    mtab = new mTableWidget(this);
    mtab->setGeometry(20,30, 351, 261);

    mtab->insertColumn(0);
    mtab->insertColumn(1);

    mtab->setColumnWidth(0, 300);
    mtab->setColumnWidth(1, 49);
    mtab->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("file")));
    mtab->setHorizontalHeaderItem(1,new QTableWidgetItem(QString("status")));
    mtab->verticalHeader()->hide();

    mtab->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mtab->setTextElideMode(Qt::ElideMiddle);


    connect(ui->addProjFile, &QPushButton::clicked, this, &mCfgDlg::addProjFile);
    connect(ui->delProjFile, &QPushButton::clicked, this, &mCfgDlg::delProjFile);
    connect(ui->saveConfig, &QPushButton::clicked, this, &mCfgDlg::saveConfigFile);
    connect(ui->saveConfigAs, &QPushButton::clicked, this, &mCfgDlg::saveConfigFileAs);
    connect(ui->cancel, &QPushButton::clicked, this, &mCfgDlg::hide);
}


bool  mCfgDlg::openCfg(QString &fileName)
{
    clear();

    curFileName = fileName;
    TiXmlDocument doc(curFileName.toLatin1().data());
    if (doc.LoadFile() == false) {
        return false;
    }

    TiXmlNode* root = doc.FirstChild("ProjectList");
    if (!root) {
        return false;
    }

    mtab->setColumnWidth(0, 300);
    for (TiXmlElement *ele = root->FirstChildElement("Project");
         ele; ele = ele->NextSiblingElement("Project")){
        QString file(ele->Attribute("Location"));
        if (isexist(file))
            continue;

        int rowCount = mtab->rowCount();
        mtab->insertRow(rowCount);
        mtab->setItem(rowCount, 0, new QTableWidgetItem(file));
        if (rowCount >= 7) {
            mtab->setColumnWidth(0, 283);
        }
    }

    return true;
}



void mCfgDlg::clear()
{
    for(int i = 0; i < mtab->rowCount();) {
        mtab->removeRow(i);
    }

    curFileName = QString();
}


void mCfgDlg::addProjFile()
{
    QFileDialog *f = new QFileDialog(this);
    f->setAcceptMode(QFileDialog::AcceptOpen);
    f->setFileMode(QFileDialog::ExistingFiles);

    f->setNameFilter(QString("*.sln;*.csproj;*.vcxproj"));
    QStringList fileNames;
    if (f->exec()) {
        fileNames = f->selectedFiles();
    }

    mtab->setColumnWidth(0, 300);
    foreach (QString filename, fileNames) {
        if (isexist(filename)) {
            continue;
        }

        int a = mtab->rowCount();
        mtab->insertRow(a);
        QTableWidgetItem *file = mtab->item(a, 0);
        if (!file)
            file = new QTableWidgetItem(filename);
        mtab->setItem(a,0, file);
        if (a >= 7) {
            mtab->setColumnWidth(0, 270);
        }
    }
}

bool mCfgDlg::isexist(QString &filename)
{
    bool flag = false;
    for(int i = 0; i < mtab->rowCount();i++) {
        if (mtab->item(i,0)->text() == filename) {
            flag = true;
            break;
        }
    }
    return flag;
}

void mCfgDlg::delProjFile()
{
    foreach( QTableWidgetItem *it, mtab->selectedItems()) {
        mtab->removeRow(it->row());
    }
    if (mtab->rowCount() >= 7)
        mtab->setColumnWidth(0, 283);
    else
        mtab->setColumnWidth(0, 300);
}

void mCfgDlg::saveConfigFile(bool flag)
{
    if (curFileName.isEmpty() || flag) {
        QFileDialog *f = new QFileDialog(this);
        f->setAcceptMode(QFileDialog::AcceptSave);
        f->setNameFilter(QString("*.cfg"));
        QStringList fileNames;
        if (f->exec()) {
            fileNames = f->selectedFiles();
        }
        if(fileNames.isEmpty()){
            return;
        }

        QString file = fileNames.at(0);
        if (file.right(4) != QString(".cfg"))
            file += ".cfg";

        curFileName = file;
    }

    TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "", "");
    TiXmlElement *listElement = new TiXmlElement("ProjectList");

    for (int i = 0; i < mtab->rowCount(); i++) {
        TiXmlElement *ele = new TiXmlElement("Project");
        ele->SetAttribute("Location", mtab->item(i,0)->text().toLatin1().data());
        listElement->LinkEndChild(ele);
    }

    TiXmlDocument doc;
    doc.LinkEndChild(dec);
    doc.LinkEndChild(listElement);
    doc.SaveFile(curFileName.toLatin1().data());

    return;
}

void mCfgDlg::saveConfigFileAs()
{
    saveConfigFile(true);
}

void mCfgDlg::on_saveResult_clicked()
{
    emit SaveResultSIG();
}
