/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGRAPHICSSCENEBSPTREE_P_H
#define QGRAPHICSSCENEBSPTREE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qlist.h>

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

#include <QtCore/qrect.h>
#include <QtCore/qset.h>
#include <QtCore/qvector.h>

class QGraphicsItem;
class QGraphicsSceneBspTreeVisitor;
class QGraphicsSceneInsertItemBspTreeVisitor;
class QGraphicsSceneRemoveItemBspTreeVisitor;
class QGraphicsSceneFindItemBspTreeVisitor;

class QGraphicsSceneBspTree
{
public:
    struct Node
    {
        enum Type { Horizontal, Vertical, Leaf };
        union {
            qreal offset;
            int leafIndex;
        };
        Type type;
    };

    QGraphicsSceneBspTree();
    ~QGraphicsSceneBspTree();

    void initialize(const QRectF &rect, int depth);
    void clear();

    void insertItem(QGraphicsItem *item, const QRectF &rect);
    void removeItem(QGraphicsItem *item, const QRectF &rect);
    void removeItems(const QSet<QGraphicsItem *> &items);

    QList<QGraphicsItem *> items(const QRectF &rect);
    QList<QGraphicsItem *> items(const QPointF &pos);
    int leafCount() const;

    inline int firstChildIndex(int index) const
    { return index * 2 + 1; }

    inline int parentIndex(int index) const
    { return index > 0 ? ((index & 1) ? ((index - 1) / 2) : ((index - 2) / 2)) : -1; }

    QString debug(int index) const;

private:
    void initialize(const QRectF &rect, int depth, int index);
    void climbTree(QGraphicsSceneBspTreeVisitor *visitor, const QPointF &pos, int index = 0);
    void climbTree(QGraphicsSceneBspTreeVisitor *visitor, const QRectF &rect, int index = 0);

    void findItems(QList<QGraphicsItem *> *foundItems, const QRectF &rect, int index);
    void findItems(QList<QGraphicsItem *> *foundItems, const QPointF &pos, int index);
    QRectF rectForIndex(int index) const;

    QVector<Node> nodes;
    QVector<QList<QGraphicsItem *> > leaves;
    int leafCnt;
    QRectF rect;

    QGraphicsSceneInsertItemBspTreeVisitor *insertVisitor;
    QGraphicsSceneRemoveItemBspTreeVisitor *removeVisitor;
    QGraphicsSceneFindItemBspTreeVisitor *findVisitor;
};

class QGraphicsSceneBspTreeVisitor
{
public:
    virtual ~QGraphicsSceneBspTreeVisitor() { }
    virtual void visit(QList<QGraphicsItem *> *items) = 0;
};

#endif // QT_NO_GRAPHICSVIEW
#endif // QGRAPHICSSCENEBSPTREE_P_H
