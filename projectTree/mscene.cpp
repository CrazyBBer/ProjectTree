#include "mscene.h"
#include "tinyxml/tinyxml.h"
#include <QDebug>


mScene::mScene(QObject* parent): QGraphicsScene(parent), curFileName()
{
}

void mScene::setLineColor(mTextItem *selected, mMap &map, const QColor &color)
{
    mMapIt it = map.find(selected);
    if (it != map.end()) {
        for(mList::iterator lineIt = it.value()->begin(); lineIt != it.value()->end(); lineIt++) {
            (*lineIt)->setPen(QPen(color));
        }
    }
}

void mScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsScene::mousePressEvent(event);

    static mTextItem *focus = NULL;

    if (focus) {    //reset QGraphicsLine colour
        setLineColor(focus, mChildMap, Qt::black);
        setLineColor(focus, mParentMap, Qt::black);
    }


    QGraphicsItem *select = focusItem();
    if (select) {
        focus = dynamic_cast<mTextItem*>(select);
        if (focus) {
            setLineColor(focus, mChildMap, Qt::green);
            setLineColor(focus, mParentMap, Qt::green);
        }
    }
}

void mScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsScene::mouseMoveEvent(event);

    foreach(QGraphicsItem* item, selectedItems()){
        mTextItem *it = dynamic_cast <mTextItem *>(item);
        updateLines(it);
    }
}

void mScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsScene::mouseReleaseEvent(event);
}

void mScene::keyPressEvent(QKeyEvent *event){
//    if(event->key() == Qt::Key_Delete)
//        foreach(QGraphicsItem* item, selectedItems()){
//            removeItem(item);
//            delete item;
//        }
//    else
        QGraphicsScene::keyPressEvent(event);
}


bool mScene::addmTextItem(mTextItem *item)
{
    //check if Item already exist
    {
        mStrMap::iterator it = itemNameMap.find(item->getName());
        if (it != itemNameMap.end()) {
            qDebug() << "Item Already exist "<<item->getName();
            return false;
        }
        itemNameMap.insert(item->getName(), item);
    }

    addItem(item);

    {   // Create Child line List
        mMapIt it = mChildMap.find(item);
        if (it == mChildMap.end()) {
            mList *l = new mList();
            if (!l) {
                qDebug() << "New Chile Line List Failed!!";
                return false;
            }
            mChildMap.insert(item, l);
        }
    }
    {   // Create Parent line List
        mMapIt it = mParentMap.find(item);
        if (it == mParentMap.end()) {
            mList *l = new mList();
            if (!l) {
                qDebug() << "New Parent Line List Failed!!";
                return false;
            }
            mParentMap.insert(item, l);
        }
    }

    return true;
}


mTextItem * mScene::addLinkedItem(mTextItem *first, const projItem *proj, LinkedItemType type, int*levelCount)
{
    bool newLink = false;
    mTextItem *second = NULL;
    {
        mStrMap::iterator it = itemNameMap.find(proj->name);
        if (it != itemNameMap.end()) {
            second = it.value();
        } else {
            int c = Qt::white;
            if (proj->type == projItem::kEXE)
                c = Qt::green;
            if (proj->path.isEmpty())
                c = Qt::yellow;
            second = new mTextItem(proj->name, (Qt::GlobalColor)c);
            if (!second)
                return NULL;
            newLink = true;
            addmTextItem(second);
        }
    }

    if (newLink) {
        second->setPos(QPoint(0+100*(levelCount[proj->level]++), 100 * proj->level));
    }

    QLineF line;
    if (type == eParentItem) {
        line.setP1(second->downHotPoint());
        line.setP2(first->upHotPoint());
    } else {
        line.setP1(first->downHotPoint());
        line.setP2(second->upHotPoint());
    }

    QGraphicsLineItem *li = new QGraphicsLineItem(line);

    addItem(li);
    {   // Add in Parent's child line list
        mMapIt it;
        if (type == eChildItem)
            it = mChildMap.find(first);
        else
            it = mChildMap.find(second);

        if (it == mChildMap.end()) {
            qDebug() << "Error!: not found Child Line List";
            return second;
        }
        it.value()->push_back(li);
    }
    {   // Add in Child's parent line list
        mMapIt it;
        if (type == eChildItem)
            it = mParentMap.find(second);
        else
            it = mParentMap.find(first);
        if (it == mParentMap.end()) {
            qDebug() << "Error!: not found Parent Line List";
            return second;
        }
        it.value()->push_back(li);
    }

    updateLines(second);


    if (type == eChildItem){
        // add child list
        foreach(projItem* childProj, proj->chilrens) {
            addLinkedItem(second, childProj, mScene::eChildItem,levelCount);
        }
    } else {
        // add parent list
        foreach(projItem* parentProj, proj->parents) {
            addLinkedItem(second, parentProj, mScene::eParentItem, levelCount);
        }
    }


    return second;
}

void mScene::updateLines(mTextItem *item)
{
    {   // Update Child Line
        mMapIt it = mChildMap.find(item);
        if (it == mChildMap.end()) {
            qDebug() << "Error!: not found mList from mChildMap";
            return;
        }
        mList *l = it.value();
        foreach(QGraphicsLineItem* li, *l) {
            QLineF line = li->line();
            line.setP1(item->downHotPoint());
            li->setLine(line);
        }
    }
    {   // Update Parent Line
        mMapIt it = mParentMap.find(item);
        if (it == mParentMap.end()) {
            qDebug() << "Error!: not found mList from mParentMap";
            return;
        }
        mList *l = it.value();
        foreach(QGraphicsLineItem* li, *l) {
            QLineF line = li->line();
            line.setP2(item->upHotPoint());
            li->setLine(line);
        }
    }

}

int mScene::getChildItemNum(mTextItem *item)
{
    mMapIt it = mChildMap.find(item);
    if (it == mChildMap.end()) {
        qDebug() << "Error!: not found Parent Line List";
        return 0;
    }
    return it.value()->size();
}


void mScene::save2file(QString fileName)
{
    mList lineItemList;
    TiXmlDocument doc;
    curFileName = fileName;

    TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "", "");
    TiXmlElement *slnElement = new TiXmlElement("QGraphicsScene");

    for (mStrMap::iterator it = itemNameMap.begin(); it != itemNameMap.end(); it++) {
        TiXmlElement *ele = new TiXmlElement("Item");
        mTextItem *item = it.value();

        ele->SetAttribute("Name", it.key().toLatin1().data());
        ele->SetAttribute("x", item->pos().x());
        ele->SetAttribute("y", item->pos().y());
        ele->SetAttribute("color", item->Color());

        {
            TiXmlElement *parentItems = new TiXmlElement("ParentItems");
            mMapIt mit = mParentMap.find(item);
            if (mit != mParentMap.end()) {
                mList *ml = mit.value();
                foreach(QGraphicsLineItem *l, *ml) {
                    TiXmlElement *lineEle = new TiXmlElement("line");
                    if (lineItemList.contains(l)) {
                        lineEle->SetAttribute("Index", lineItemList.indexOf(l));
                    } else {
                        lineEle->SetAttribute("Index", lineItemList.size());
                        lineItemList.push_back(l);
                    }
                    parentItems->LinkEndChild(lineEle);
                }
            }
            ele->LinkEndChild(parentItems);
        }
        {
            TiXmlElement *childItems = new TiXmlElement("ChildItems");
            mMapIt mit = mChildMap.find(item);
            if (mit != mChildMap.end()) {
                mList *ml = mit.value();
                foreach(QGraphicsLineItem *l, *ml) {
                    TiXmlElement *lineEle = new TiXmlElement("line");
                    if (lineItemList.contains(l)) {
                        lineEle->SetAttribute("Index", lineItemList.indexOf(l));
                    } else {
                        lineEle->SetAttribute("Index", lineItemList.size());
                        lineItemList.push_back(l);
                    }
                    childItems->LinkEndChild(lineEle);
                }
            }
            ele->LinkEndChild(childItems);
        }

        slnElement->LinkEndChild(ele);
    }

    doc.LinkEndChild(dec);
    doc.LinkEndChild(slnElement);
    doc.SaveFile(fileName.toLatin1().data());
}

bool mScene::loadfromfile(QString fileName)
{
    sceneReset();
    curFileName = fileName;

    mList lineItemList;
    TiXmlDocument doc(fileName.toLatin1().data());
    if (doc.LoadFile() == false) {
        qDebug()<<"loadFile failed";
        return false;
    }

    TiXmlNode* root = doc.FirstChild("QGraphicsScene");
    if (!root) {
        return false;
    }

    for (TiXmlElement *ele = root->FirstChildElement("Item");
         ele; ele = ele->NextSiblingElement("Item")){
        QString name(ele->Attribute("Name"));
        if (itemNameMap.contains(name))
            continue;

        int x,y, color;
        ele->QueryIntAttribute("x", &x);
        ele->QueryIntAttribute("y", &y);
        ele->QueryIntAttribute("color", &color);
        QPoint pos(x, y);
        mTextItem *item = new mTextItem(name, (Qt::GlobalColor)color);
        if (!item)
            return false;

        if(addmTextItem(item) == false)
            return false;

        item->setPos(pos);
        {
            TiXmlElement *parentItems = ele->FirstChildElement("ParentItems");
            if (!parentItems)
                return false;
            mMapIt mit = mParentMap.find(item);
            mList *ml = mit.value();
            for (TiXmlElement *lineEle = parentItems->FirstChildElement("line");
                 lineEle;
                 lineEle = lineEle->NextSiblingElement("line")) {
                int index;
                lineEle->QueryIntAttribute("Index", &index);
                if (lineItemList.size() == index) {
                    QLineF line(QPointF(0,0), item->downHotPoint());
                    QGraphicsLineItem *l = new QGraphicsLineItem(line);
                    if (!l)
                        return false;
                    addItem(l);

                    ml->push_back(l);
                    lineItemList.push_back(l);
                } else {
                    QGraphicsLineItem *l = lineItemList.at(index);
                    ml->push_back(l);
                }
            }
        }
        {
            TiXmlElement *childItems = ele->FirstChildElement("ChildItems");
            if (!childItems)
                return false;
            mMapIt mit = mChildMap.find(item);
            mList *ml = mit.value();
            for (TiXmlElement *lineEle = childItems->FirstChildElement("line");
                 lineEle;
                 lineEle = lineEle->NextSiblingElement("line")) {
                int index;
                lineEle->QueryIntAttribute("Index", &index);
                if (lineItemList.size() <= index) {
                    QLineF line(item->upHotPoint(), QPointF(0,0));
                    QGraphicsLineItem *l = new QGraphicsLineItem(line);
                    if (!l)
                        return false;
                    addItem(l);

                    ml->push_back(l);
                    lineItemList.push_back(l);
                } else {
                    QGraphicsLineItem *l = lineItemList.at(index);
                    ml->push_back(l);
                }
            }
        }
        updateLines(item);
    }
    return true;
}

void mScene::sceneReset()
{
    clear();

    mChildMap.clear();
    mParentMap.clear();
    itemNameMap.clear();
}
