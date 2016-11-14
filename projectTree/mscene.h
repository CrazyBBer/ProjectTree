#ifndef MSCENE_H
#define MSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QAction>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMap>
#include <QList>
#include "mTextItem.h"
#include "projitem.h"


typedef QList<QGraphicsLineItem*>  mList;
typedef QMap<mTextItem*, mList*> mMap;
typedef QMap<mTextItem*, mList*>::iterator mMapIt;
typedef QMap<QString, mTextItem*>   mStrMap;


class mScene : public QGraphicsScene
{
public:
    enum Mode {NoMode, SelectObject, DrawLine};
    enum LinkedItemType {
        eParentItem,
        eChildItem
    };

    mScene(QObject* parent = 0);
    bool addmTextItem(mTextItem *item);
    mTextItem *addchilditem(mTextItem *item, const QString name);
    mTextItem *addLinkedItem(mTextItem *first, const projItem *proj, LinkedItemType type, int *levelCount);

    void sceneReset();
    void save2file(QString fileName);
    bool loadfromfile(QString fileName);

    QString getCurFileName() const {
        return curFileName;
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    void updateLines(mTextItem *item);
    int getChildItemNum(mTextItem *item);
    void setLineColor(mTextItem *selected, mMap &map, const QColor &color);

private:
    mMap mChildMap;
    mMap mParentMap;
    mStrMap itemNameMap;

    QString curFileName;
};

#endif // MSCENE_H
