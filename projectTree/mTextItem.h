#ifndef BUTTITEM_H
#define BUTTITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPoint>
#include <QLine>



typedef QList<QLine*> llist_t;
typedef QList<QLine*>::iterator llit;

class mTextItem: virtual public QGraphicsItem
{
public:
   explicit  mTextItem(QString n, Qt::GlobalColor c = Qt::white) :name(n), color(c) {
        int len = name.length();
        QPoint lu(-3, -2);
        QPoint ld(-3, 15);
        QPoint ru(len * 6+4, -2);
        QPoint rd(len * 6+4, 15);

        left = new QLine(lu, ld);
        right = new QLine(ru, rd);
        up = new QLine(lu, ru);
        down = new QLine(ld, rd);

        rec = new QRect(lu,rd);

        setFlag(QGraphicsItem::ItemIsMovable,true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsFocusable, true);
    }

    enum { Type = UserType + 1 };

    int type() const { return Type; }

    QRectF boundingRect() const
    {
        return *rec;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        //tolocal8bit
        Q_UNUSED(option);
        Q_UNUSED(widget);

        painter->fillRect(*rec, QColor(color));
        painter->drawLine(*left);
        painter->drawLine(*right);
        painter->drawLine(*down);
        painter->drawLine(*up);
        painter->drawText(QRect(0, 0, name.length()*6, 15), name);
    }

    QPointF upHotPoint()  {
        return QPointF(pos() + (up->p1()+up->p2())/2);
    }
    QPointF downHotPoint() const {
        return QPointF(pos() + (down->p1()+down->p2())/2);
    }
    QString getName() const {
        return name;
    }
    int width() const {
        return name.length() * 6+4;
    }
    int Color() const {
        return color;
    }

private:
    QString name;
    QLine   *left;
    QLine   *right;
    QLine   *up;
    QLine   *down;
    QRect   *rec;
    Qt::GlobalColor  color;

};

#endif // BUTTITEM_H

