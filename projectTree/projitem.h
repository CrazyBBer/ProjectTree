#ifndef PROJITEM_H
#define PROJITEM_H

#include <QString>
#include <QList>


class projItem
{
public:
    projItem(QString &n, QString &p = QString()):name(n), path(p), flag(false), level(0), type(kUnknown)
    {
    }

    enum projType {
        kDLL=0,
        kLIB,
        kEXE,
        kUnknown
    };

    bool    flag;
    int     type;
    int     level;
    QString name;
    QString path;
    QList<projItem *> parents;
    QList<projItem *> chilrens;
};


typedef QList<projItem *> projItemList;


#endif // PROJITEM_H
